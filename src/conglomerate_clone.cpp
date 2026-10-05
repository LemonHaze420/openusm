#include "conglomerate_clone.h"

#include "common.h"
#include "conglom.h"
#include "distance_fader.h"
#include "entity_mash.h"
#include "memory.h"
#include "oldmath_po.h"
#include "parse_generic_mash.h"
#include "vtbl.h"
#include "wds.h"

#include <algorithm>
#include <array>

VALIDATE_SIZE(conglomerate_clone, 0xD0);
VALIDATE_OFFSET(conglomerate_clone, field_C0, 0xC0);
VALIDATE_OFFSET(conglomerate_clone, field_C4, 0xC4);
VALIDATE_OFFSET(conglomerate_clone, field_C8, 0xC8);
VALIDATE_OFFSET(conglomerate_clone, field_CC, 0xCC);

namespace {
void *__fastcall clone_delete(conglomerate_clone *self, void *, unsigned flags)
{
    self->~conglomerate_clone();
    if (flags & 1)
        mem_dealloc(self, sizeof(conglomerate_clone));
    return self;
}
int __fastcall clone_size(conglomerate_clone *, void *) { return sizeof(conglomerate_clone); }
int __fastcall clone_flavor(conglomerate_clone *, void *) { return 13; }
bool __fastcall clone_query(conglomerate_clone *, void *) { return true; }
void __fastcall clone_release(conglomerate_clone *self, void *) { self->release_mem(); }
float __fastcall clone_radius(conglomerate_clone *self, void *) { return self->_get_visual_radius(); }
vector3d *__fastcall clone_center(conglomerate_clone *self, void *, vector3d *out)
{
    *out = self->_get_visual_center();
    return out;
}
bool __fastcall clone_renderable(conglomerate_clone *self, void *) { return self->_is_renderable(); }
void __fastcall clone_render(conglomerate_clone *self, void *, Float fade) { self->_render(fade); }
void __fastcall clone_unmash(conglomerate_clone *self, void *, generic_mash_header *header,
                             void *object, generic_mash_data_ptrs *data)
{
    self->un_mash(header, object, data);
}
void __fastcall clone_unlock_ifl(conglomerate_clone *self, void *) { self->field_90.field_6 |= 0x3FFF; }
void __fastcall clone_lock_ifl(conglomerate_clone *self, void *, uint16_t frame)
{
    self->field_90.field_6 ^= (frame ^ self->field_90.field_6) & 0x3FFF;
}

void __fastcall clone_release_ifl(conglomerate_clone *, void *) {}
}

void *conglomerate_clone::native_vtable(void **actor_table)
{

    static std::array<void *, 0x294 / 4> table;
    std::copy_n(actor_table, table.size(), table.begin());
    table[0] = reinterpret_cast<void *>(&clone_delete);
    table[0x4 / 4] = reinterpret_cast<void *>(&clone_size);
    table[0x10 / 4] = reinterpret_cast<void *>(&clone_release);
    table[0x28 / 4] = reinterpret_cast<void *>(&clone_radius);
    table[0x2C / 4] = reinterpret_cast<void *>(&clone_center);
    table[0x54 / 4] = reinterpret_cast<void *>(&clone_flavor);
    table[0x68 / 4] = reinterpret_cast<void *>(&clone_query);
    table[0x164 / 4] = reinterpret_cast<void *>(&clone_unmash);
    table[0x18C / 4] = reinterpret_cast<void *>(&clone_renderable);
    table[0x1AC / 4] = reinterpret_cast<void *>(&clone_render);
    table[0x264 / 4] = reinterpret_cast<void *>(&clone_unlock_ifl);
    table[0x268 / 4] = reinterpret_cast<void *>(&clone_lock_ifl);
    table[0x26C / 4] = reinterpret_cast<void *>(&clone_release_ifl);
    return table.data();
}

conglomerate_clone::conglomerate_clone(const string_hash &id, uint32_t flags) : actor(id, flags)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[6];
#else
    m_vtbl = 0x00884438;
#endif
    field_C0 = INVALID_HANDLE;
    field_C4 = nullptr;
    field_C8 = nullptr;
    field_CC = nullptr;
}

conglomerate_clone::~conglomerate_clone()
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[6];
#else
    m_vtbl = 0x00884438;
#endif
    common_destruct();
}

void conglomerate_clone::common_destruct()
{

    if (field_C8 != nullptr) {
        if (--*field_C8 <= 0) {
            if (auto *source = field_C0.get_volatile_ptr())
                g_world_ptr->ent_mgr.destroy_entity(static_cast<entity *>(source));
            *field_CC = INVALID_HANDLE;
        }
    }
    field_C0 = INVALID_HANDLE;
    field_C4 = nullptr;
    field_CC = nullptr;
}

void conglomerate_clone::release_mem()
{
    common_destruct();
    actor::release_mem();
}

void conglomerate_clone::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    actor::un_mash(header, object, data);
    field_C0 = INVALID_HANDLE;
    field_C4 = nullptr;
    field_C8 = nullptr;
    field_CC = nullptr;
    data->rebase_shared(8);
    field_C4 = data->get_from_shared<resource_key>();
    data->rebase_shared(4);
    field_C8 = data->get_from_shared<int>();
    data->rebase_shared(4);
    field_CC = data->get_from_shared<entity_base_vhandle>();
    if (*field_C8 == 0) {
        auto *source = static_cast<conglomerate *>(g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
            field_C4->m_hash, make_unique_entity_id(), po_identity_matrix, mString{""}, 0xA1, nullptr));
        source->remove_from_regions();
        source->field_4 &= ~0x200u;
        source->field_110 |= 0x40;
        *field_CC = source->my_handle;
    }
    ++*field_C8;
    field_C0 = *field_CC;
    if (!is_flagged(0x200000))
        on_fade_distance_changed(distance_fader::estimate_fade_index_for_bounding_sphere(_get_visual_radius()));
}

float conglomerate_clone::_get_visual_radius()
{
    if (auto *source = field_C0.get_volatile_ptr())
        return source->get_visual_radius();
    return actor::_get_visual_radius();
}

vector3d conglomerate_clone::_get_visual_center()
{
    const auto flags = field_8;
    const auto position = get_abs_position();
    return position + sub_509170(this, flags);
}

bool conglomerate_clone::_is_renderable()
{
    if (auto *source = field_C0.get_volatile_ptr())
        return static_cast<entity *>(source)->is_renderable();
    return false;
}

void conglomerate_clone::_render(Float fade)
{
    auto *source = static_cast<conglomerate *>(field_C0.get_volatile_ptr());
    if (source == nullptr)
        return;
    for (auto *member : source->members) {
        if ((member->field_4 & 0x300) != 0x300)
            continue;
        const auto saved = member->get_abs_po();
        po transformed;
        transformed.set_from_ptr_to_po_world(ptr_to_po{&saved.m, &my_abs_po->m});
        *member->my_abs_po = transformed;
        static_cast<entity *>(member)->render(fade);
        *member->my_abs_po = saved;
    }
}
