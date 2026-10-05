#include "handheld_item.h"

#include "color32.h"
#include "common.h"
#include "entity_mash.h"
#include "memory.h"
#include "parse_generic_mash.h"
#include "physical_interface.h"
#include "vtbl.h"
#include "wds.h"

#include <algorithm>
#include <array>
#include <new>

VALIDATE_SIZE(handheld_item, 0x114);

namespace {

struct attachment_data {
    string_hash drawn_bone;
    string_hash holstered_bone;
    unsigned char field_8[0x1C];
    float drawn_scale;
    vector3d drawn_rotation;
    vector3d drawn_position;
    vector3d holstered_rotation;
    vector3d holstered_position;
    float holstered_scale;
    uint16_t field_5C;
    uint16_t flags;
};
VALIDATE_SIZE(attachment_data, 0x60);

void *__fastcall handheld_delete(handheld_item *self, void *, unsigned flags)
{
    self->~handheld_item();
    if (flags & 1)
        mem_dealloc(self, sizeof(handheld_item));
    return self;
}
int __fastcall handheld_size(handheld_item *, void *) { return sizeof(handheld_item); }
void __fastcall handheld_release(handheld_item *self, void *) { self->release_mem(); }
bool __fastcall handheld_chunk(handheld_item *, void *, void *, void *) { return false; }
bool __fastcall handheld_query(handheld_item *, void *) { return true; }
void __fastcall handheld_unmash(handheld_item *self, void *, generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    self->un_mash(header, object, data);
}
void __fastcall handheld_advance(handheld_item *self, void *, Float elapsed) { self->frame_advance(elapsed); }
void __fastcall handheld_holster(handheld_item *self, void *, bool visible) { self->holster(visible); }
void __fastcall handheld_draw(handheld_item *self, void *, bool visible) { self->draw(visible); }
void __fastcall handheld_hide(handheld_item *self, void *) { self->hide(); }
void __fastcall handheld_show(handheld_item *self, void *) { self->show(); }

void __fastcall handheld_idle(handheld_item *, void *) {}
void __fastcall handheld_effect(handheld_item *, void *, int, int, int) {}
bool __fastcall handheld_available(handheld_item *self, void *) { return !(self->field_10C & 0x10); }
entity_base *__fastcall handheld_owner(handheld_item *self, void *) { return self->field_108; }
void __fastcall handheld_set_owner(handheld_item *self, void *, actor *owner) { self->set_owner(owner); }
void __fastcall handheld_create_visual(handheld_item *self, void *) { self->create_visual_item(); }
void __fastcall handheld_visibility(handheld_item *self, void *, bool visible) { self->set_visibility(visible); }
void __fastcall handheld_detach(handheld_item *self, void *) { self->detach(); }

visual_item *ensure_visual(handheld_item *self)
{
    auto create = reinterpret_cast<void (__fastcall *)(handheld_item *, void *)>(get_vfunc(self->m_vtbl, 0x2E4));
    create(self, nullptr);
    return self->field_104.get_volatile_ptr();
}
}

void *handheld_item::native_vtable(void **item_table)
{

    static std::array<void *, 0x2F8 / 4> table;
    std::copy_n(item_table, 0x2C4 / 4, table.begin());
    table[0] = reinterpret_cast<void *>(&handheld_delete);
    table[1] = reinterpret_cast<void *>(&handheld_size);
    table[0x10 / 4] = reinterpret_cast<void *>(&handheld_release);
    table[0x20 / 4] = reinterpret_cast<void *>(&handheld_chunk);
    table[0xDC / 4] = reinterpret_cast<void *>(&handheld_query);
    table[0x164 / 4] = reinterpret_cast<void *>(&handheld_unmash);
    table[0x1A4 / 4] = reinterpret_cast<void *>(&handheld_advance);
    table[0x2C4 / 4] = reinterpret_cast<void *>(&handheld_holster);
    table[0x2C8 / 4] = reinterpret_cast<void *>(&handheld_draw);
    table[0x2CC / 4] = reinterpret_cast<void *>(&handheld_hide);
    table[0x2D0 / 4] = reinterpret_cast<void *>(&handheld_show);
    table[0x2D4 / 4] = reinterpret_cast<void *>(&handheld_idle);
    table[0x2D8 / 4] = reinterpret_cast<void *>(&handheld_available);
    table[0x2DC / 4] = reinterpret_cast<void *>(&handheld_owner);
    table[0x2E0 / 4] = reinterpret_cast<void *>(&handheld_set_owner);
    table[0x2E4 / 4] = reinterpret_cast<void *>(&handheld_create_visual);
    table[0x2E8 / 4] = reinterpret_cast<void *>(&handheld_visibility);
    table[0x2EC / 4] = reinterpret_cast<void *>(&handheld_detach);
    table[0x2F0 / 4] = reinterpret_cast<void *>(&handheld_effect);
    table[0x2F4 / 4] = reinterpret_cast<void *>(&handheld_effect);
    return table.data();
}

handheld_item::~handheld_item() = default;

void handheld_item::release_mem()
{
    if (auto *visual = field_104.get_volatile_ptr()) {
        g_world_ptr->ent_mgr.destroy_entity(visual);
        field_104.field_0 = INVALID_HANDLE;
    }
    auto *data = reinterpret_cast<attachment_data *>(field_100);
    if (data && (data->flags & 0x8000))
        ::operator delete(data);
    field_100 = 0;
    item::release_mem();
}

void handheld_item::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    field_108 = nullptr;
    field_104.field_0 = INVALID_HANDLE;
    field_10C &= ~1;
    field_110 = false;
    item::un_mash(header, object, data);
    data->rebase_shared(8);
    data->rebase_shared(4);
    auto *attachments = data->get_from_shared<attachment_data>();
    field_100 = reinterpret_cast<int>(attachments);
    attachments->flags &= ~0x8000u;
}

void handheld_item::set_owner(actor *owner)
{
    field_108 = owner;
    if (auto *visual = field_104.get_volatile_ptr())
        visual->field_C4 = owner;
}

void handheld_item::create_visual_item()
{
    if (field_104.get_volatile_ptr())
        return;
    auto *visual = ::new (mem_alloc(sizeof(visual_item))) visual_item(make_unique_entity_id(), 0);
    field_104.field_0 = visual->my_handle;
    auto get_mesh = reinterpret_cast<nglMesh *(__fastcall *)(handheld_item *, void *)>(get_vfunc(m_vtbl, 0x1B0));
    visual->field_90.set_mesh(get_mesh(this, nullptr));
    if (visual->_get_mesh()) visual->field_8 |= 0x100;
    else visual->field_8 &= ~0x100u;
    visual->set_abs_position(field_108->get_abs_position());
    g_world_ptr->ent_mgr.add_dynamic_instanced_entity(visual);
    visual->field_C4 = field_108;
    if (visual->is_renderable())
        visual->on_fade_distance_changed(field_4 & 0xF);
}

void handheld_item::draw(bool visible)
{
    auto *visual = ensure_visual(this);
    const auto &data = *reinterpret_cast<attachment_data *>(field_100);
    if (field_108 && field_108->get_flavor() == 12) {
        visual->attach(field_108, data.drawn_bone, data.drawn_scale, data.drawn_position, data.drawn_rotation, true);
        field_110 = visible && visual->is_renderable();
        visual->set_visible(field_110, false);
        visual->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    } else {
        if (field_108)
            visual->attach(field_108, string_hash{}, data.drawn_scale, data.drawn_position, data.drawn_rotation, true);
        if (visible) {
            visual->set_visible(false, false);
            field_110 = false;
        }
    }
    field_10C |= 1;
}

void handheld_item::holster(bool visible)
{
    auto *visual = ensure_visual(this);
    const auto &data = *reinterpret_cast<attachment_data *>(field_100);
    if (data.holstered_bone == string_hash{}) {
        visual->set_visible(false, false);
        field_110 = false;
    } else if (field_108 && field_108->get_flavor() == 12) {
        visual->attach(field_108, data.holstered_bone, data.holstered_scale, data.holstered_position, data.holstered_rotation, false);
        visual->set_visible(visible, false);
        visual->compute_sector(g_world_ptr->the_terrain, false, nullptr);
        field_110 = visible;
    } else {
        if (field_108)
            visual->attach(field_108, string_hash{}, data.holstered_scale, data.holstered_position, data.holstered_rotation, false);
        if (visible) {
            visual->set_visible(false, false);
            field_110 = false;
        }
    }
    field_10C &= ~1;
}

void handheld_item::show()
{
    ensure_visual(this)->set_visible(true, false);
    field_110 = true;
}
void handheld_item::hide()
{
    ensure_visual(this)->set_visible(false, false);
    field_110 = false;
}
void handheld_item::set_visibility(bool visible)
{
    ensure_visual(this)->set_visible(visible && field_110, false);
}
void handheld_item::detach()
{
    auto *visual = ensure_visual(this);
    visual->clear_parent(true);
    visual->set_visible(false, false);
}

void handheld_item::frame_advance(Float elapsed)
{
    item::frame_advance(elapsed);
    auto *visual = field_104.get_volatile_ptr();
    if (!visual)
        return;
    if (field_108) {
        if (m_physical_interface)
            m_physical_interface->field_C &= ~1u;
        visual->compute_sector(g_world_ptr->the_terrain, false, nullptr);
        visual->set_visible((field_108->field_8 & 0x200) && field_110, false);
        auto color = visual->_get_render_color();
        auto get_color = reinterpret_cast<color32 *(__fastcall *)(entity_base *, void *, color32 *)>(get_vfunc(field_108->m_vtbl, 0x1C4));
        color32 owner_color;
        get_color(field_108, nullptr, &owner_color);
        color[3] = owner_color[3];
        visual->_set_render_color(color);
    } else {
        if (m_physical_interface) {
            if (m_physical_interface->field_C & 1) {
                const auto velocity = m_physical_interface->get_velocity();
                if (velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z <= 0.0625f) {
                    m_physical_interface->field_C &= ~1u;
                    visual->compute_sector(g_world_ptr->the_terrain, false, nullptr);
                }
            }
            set_visible((m_physical_interface->field_C & 1) != 0, false);
        } else {
            set_visible(false, false);
        }
        visual->set_visible(false, false);
        visual->_set_render_color(_get_render_color());
        visual->clear_parent(true);
        visual->set_abs_po(get_rel_po());
    }
}
