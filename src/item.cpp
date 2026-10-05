#include "item.h"

#include "base_ai_data.h"
#include "advanced_entity_ptrs.h"
#include "damage_interface.h"
#include "common.h"
#include "dynamic_rtree.h"
#include "entity_mash.h"
#include "parse_generic_mash.h"
#include <new>
#include "signaller.h"
#include "time_interface.h"
#include "vtbl.h"
#include "wds.h"
#include "script.h"
#include "script_object.h"
#include "oldmath_po.h"
#include "event.h"
#include "event_manager.h"
#include <algorithm>

VALIDATE_SIZE(item, 0x100u);

Var<item *> item::inactive_items{0x0095A730};

Var<item *> item::active_items{0x0095A734};

item::item(const string_hash &a2, uint32_t a3) : actor(a2, a3)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[8];
#else
    m_vtbl = 0x00884DC0;
#endif
    field_C0 = nullptr;
    field_C4 = nullptr;
    this->field_C8 = false;
    this->field_C9 = false;
    this->field_CA = false;
    this->field_CB = false;
    this->field_CC = false;
    this->field_D4 = {};
    this->field_F4 = 0.0;
}

item::~item()
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[8];
#else
    m_vtbl = 0x00884DC0;
#endif
}

void item::frame_advance_all_items(Float elapsed)
{
    for (auto *current = active_items(); current != nullptr;) {
        auto *next = current->field_C0;
        const float scale = current->field_58 != nullptr
            ? static_cast<float>(current->field_58->sub_4ADE50())
            : g_world_ptr->time_manager.field_0;
        if (current->m_vtbl != 0) {
            auto *address = get_vfunc(current->m_vtbl, 0x1A4);
            if (address != nullptr) {
                void(__fastcall *frame_advance)(item *, void *, Float) =
                    CAST(frame_advance, address);
                frame_advance(current, nullptr, Float{scale * elapsed.value});
            }
        }
        current = next;
    }
}

bool item::is_same_item(const item &a2)
{
    bool result;

    auto v2 = this->field_D0;
    if (v2 != a2.field_D0) {
        return false;
    }

    switch (v2) {
    case 1:
    case 2:
    case 9:
        return false;
    case 3:
    case 4:
        result = this->field_D4.operator==(a2.field_D4.c_str());
        break;
    default:
        result = true;
        break;
    }

    return result;
}

void item::remove_from_list()
{
    auto *v1 = this->field_C4;
    if (v1 != nullptr) {
        v1->field_C0 = this->field_C0;
    } else if (item::inactive_items() == this) {
        item::inactive_items() = this->field_C0;
    } else if (item::active_items() == this) {
        item::active_items() = this->field_C0;
    }

    auto *v2 = this->field_C0;
    if (v2 != nullptr) {
        v2->field_C4 = this->field_C4;
    }

    this->field_C0 = nullptr;
    this->field_C4 = nullptr;
}

bool item::give_to_entity(actor *owner)
{
    bool unique = field_D0 == 0;
    if (field_D0 == 1 || field_D0 == 2 || field_D0 == 3 || field_D0 == 4 || field_D0 == 9) {
        unique = true;
        if (owner->adv_ptrs && owner->adv_ptrs->coninfo) {
            for (const auto &handle : owner->adv_ptrs->coninfo->items) {
                if (auto *value = handle.get_volatile_ptr(); value && value->is_same_item(*this)) {
                    unique = false;
                    break;
                }
            }
        }
    }
    if (unique)
        spawn_item_script();
    auto quantity = reinterpret_cast<int (__fastcall *)(item *, void *)>(get_vfunc(m_vtbl, 0x2AC));
    auto set = reinterpret_cast<void (__fastcall *)(item *, void *, int)>(get_vfunc(m_vtbl, 0x2A4));
    auto apply = reinterpret_cast<void (__fastcall *)(item *, void *, actor *)>(get_vfunc(m_vtbl, 0x2B8));
    const int before = quantity(this, nullptr);
    bool received = false;
    switch (field_D0) {
    case 0:
        apply(this, nullptr, owner);
        set(this, nullptr, 0);
        unique = false;
        break;
    case 1: case 2: case 3: case 4: {
        auto add = reinterpret_cast<bool (__fastcall *)(actor *, void *, entity_base_vhandle, bool)>(get_vfunc(owner->m_vtbl, 0x288));
        received = add(owner, nullptr, my_handle, true);
        break;
    }
    case 6: case 7: {
        auto has_damage = reinterpret_cast<bool (__fastcall *)(actor *, void *)>(get_vfunc(owner->m_vtbl, 0x114));
        if (has_damage(owner, nullptr)) {
            auto get_damage = reinterpret_cast<damage_interface *(__fastcall *)(actor *, void *)>(get_vfunc(owner->m_vtbl, 0x118));
            auto *damage = get_damage(owner, nullptr);
            auto &value = field_D0 == 6 ? damage->field_1FC : damage->field_20C;
            value.field_0[0] = std::clamp(value.field_0[0] + static_cast<float>(field_E4),
                                         value.field_0[1], value.field_0[2]);
            set(this, nullptr, 0);
        }
        break;
    }
    case 9:
        received = true;
        apply(this, nullptr, owner);
        set(this, nullptr, 0);
        break;
    }
    if (before != quantity(this, nullptr) || received)
        event_manager::raise_event(event::PICKUP, my_handle);
    auto remaining = reinterpret_cast<int (__fastcall *)(item *, void *)>(get_vfunc(m_vtbl, 0x298));
    if (unique || !remaining(this, nullptr))
        field_CB = true;
    auto is_hero = reinterpret_cast<bool (__fastcall *)(actor *, void *)>(get_vfunc(owner->m_vtbl, 0x4C));
    if (is_hero(owner, nullptr) && adv_ptrs && adv_ptrs->coninfo) {

        for (const auto &handle : adv_ptrs->coninfo->items) {
            if (auto *value = handle.get_volatile_ptr()) {
                auto count = reinterpret_cast<int (__fastcall *)(item *, void *)>(get_vfunc(value->m_vtbl, 0x2AC));
                if (count(value, nullptr) > 0) {
                    auto add = reinterpret_cast<bool (__fastcall *)(actor *, void *, entity_base_vhandle, bool)>(get_vfunc(owner->m_vtbl, 0x288));
                    add(owner, nullptr, value->my_handle, true);
                }
            }
        }
        adv_ptrs->coninfo->items._Tidy();
    }
    return unique;
}

void item::release_mem()
{
    this->remove_from_list();
    this->field_D4.~mString();
    auto v2 = this->field_7C;
    if (v2 != nullptr) {
        v2->destruct_mashed_class();
    }

    collision_dynamic_rtree().remove_entity(this);
    actor::common_destruct();
    entity::release_mem();
}

void item::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    actor::_un_mash(header, object, data);
    field_C8 = false;
    field_C9 = false;
    field_CC = false;
    field_C0 = nullptr;
    field_C4 = nullptr;
    field_CB = false;
    auto show_family = reinterpret_cast<void(__fastcall *)(entity *, void *, bool)>(
        get_vfunc(m_vtbl, 0x188));
    show_family(this, nullptr, true);
    remove_from_list();
    field_C0 = active_items();
    active_items() = this;
    if (field_C0 != nullptr)
        field_C0->field_C4 = this;
    ::new (static_cast<void *>(&field_D4)) mString{};
    data->rebase_shared(4);
    const int size = *data->get_from_shared<int>();
    field_D4 = reinterpret_cast<const char *>(data->get_from_shared<uint8_t>(size));
}

void item::change_list_status()
{
    remove_from_list();
    auto is_handheld = reinterpret_cast<bool (__fastcall *)(item *, void *)>(get_vfunc(m_vtbl, 0xDC));
    auto &head = (!field_CB || is_handheld(this, nullptr)) ? active_items() : inactive_items();
    field_C0 = head;
    head = this;
    if (field_C0)
        field_C0->field_C4 = this;
}

void item::spawn_item_script()
{
    auto *global = script::get_gso();
    if (!global || field_C9)
        return;
    field_C9 = true;
    auto name = field_D4 + "_callbacks(item)";
    name.to_lower();
    const int index = global->find_func(string_hash{name.c_str()});
    if (index >= 0) {
        auto *self = this;
        auto *instance = script::get_gsoi();
        auto *thread = instance->add_thread(global->get_func(index), reinterpret_cast<const char *>(&self));
        instance->run_single_thread(thread, false);
    }
}

void item::set_quantity(int quantity)
{
    field_E4 = quantity;
    if (field_CB && quantity) {
        set_family_visible(true);
        compute_sector(g_world_ptr->the_terrain, false, nullptr);
        field_CB = false;
        field_F4 = 0.0f;
    } else if (!field_CB && !quantity) {
        set_family_visible(false);
        field_CB = true;
        field_F4 = 1.0f;
    }
}

void item::frame_advance(Float elapsed)
{
    if (field_CB) {
        field_F4 = 0.0f;
        if (field_8 & 0x200)
            set_family_visible(false);
        return;
    }
    if (!(field_8 & 0x200))
        set_family_visible(true);
    if (!field_CC)
        field_F4 = std::max(0.0f, field_F4 - elapsed.value);
    if (!get_occluded_last_frame()) {
        po rotation = po_identity_matrix;
        rotation.set_rotate_y(Float{3.1415927f * elapsed.value});
        po pose = get_rel_po();
        const vector3d position = pose.get_position();
        pose.set_position(ZEROVEC);
        pose.set_from_ptr_to_po_world(ptr_to_po{&pose.m, &rotation.m});
        pose.sub_48D840();
        if (!(field_EC <= 1.0f && field_EC >= 1.0f))
            pose.m.scale(Float{field_EC});
        pose.set_position(position);
        set_abs_po(pose);
    }
}

void item::apply_effects(actor *)
{
    event_manager::raise_event(event::USE, my_handle);
}

namespace {
void __fastcall native_item_quantity(item *self, void *, int value) { self->set_quantity(value); }
int __fastcall native_item_get_quantity(item *self, void *) { return self->field_E4; }
void __fastcall native_item_advance(item *self, void *, Float elapsed) { self->frame_advance(elapsed); }
void __fastcall native_item_spawn_script(item *self, void *) { self->spawn_item_script(); }
bool __fastcall native_item_not_handheld(item *, void *) { return false; }
void __fastcall native_item_increment(item *self, void *) { ++self->field_E4; }
void __fastcall native_item_decrement(item *self, void *) { --self->field_E4; }
bool __fastcall native_item_has_quantity(item *self, void *)
{
    auto quantity = reinterpret_cast<int (__fastcall *)(item *, void *)>(get_vfunc(self->m_vtbl, 0x298));
    return quantity(self, nullptr) > 0;
}
void __fastcall native_item_apply(item *self, void *, actor *owner) { self->apply_effects(owner); }
int __fastcall native_item_size(item *, void *) { return sizeof(item); }
bool __fastcall native_item_chunk(item *, void *, void *, void *) { return false; }
bool __fastcall native_item_query(item *, void *) { return true; }
float __fastcall native_item_radius(item *self, void *) { return std::max(0.25f, self->_get_visual_radius()); }
void __fastcall native_item_render(item *self, void *, Float fade)
{
    if (self->field_E4 > 0)
        self->actor::_render(fade);
}
bool __fastcall native_item_give(item *self, void *, actor *owner) { return self->give_to_entity(owner); }
bool __fastcall native_item_health(item *self, void *) { return self->field_D0 == 6; }
bool __fastcall native_item_armor(item *self, void *) { return self->field_D0 == 7; }
void __fastcall native_item_preload(item *self, void *)
{
    if (self->field_C8)
        return;
    self->field_C8 = true;
    auto name = self->field_D4 + "_preload()";
    name.to_lower();
    auto *global = script::get_gso();
    const int index = global->find_func(string_hash{name.c_str()});
    if (index >= 0) {
        auto *instance = script::get_gsoi();
        instance->run_single_thread(instance->add_thread(global->get_func(index)), false);
    }
}
}

void item::install_weapon_callbacks(void **table)
{
    table[0x04 / 4] = reinterpret_cast<void *>(&native_item_size);
    table[0x20 / 4] = reinterpret_cast<void *>(&native_item_chunk);
    table[0x28 / 4] = reinterpret_cast<void *>(&native_item_radius);
    table[0xD8 / 4] = reinterpret_cast<void *>(&native_item_query);
    table[0x1AC / 4] = reinterpret_cast<void *>(&native_item_render);
    table[0xDC / 4] = reinterpret_cast<void *>(&native_item_not_handheld);
    table[0xE0 / 4] = reinterpret_cast<void *>(&native_item_not_handheld);
    table[0xE4 / 4] = reinterpret_cast<void *>(&native_item_not_handheld);
    table[0xE8 / 4] = reinterpret_cast<void *>(&native_item_not_handheld);
    table[0x1A4 / 4] = reinterpret_cast<void *>(&native_item_advance);
    table[0x294 / 4] = reinterpret_cast<void *>(&native_item_spawn_script);
    table[0x298 / 4] = reinterpret_cast<void *>(&native_item_get_quantity);
    table[0x29C / 4] = reinterpret_cast<void *>(&native_item_increment);
    table[0x2A0 / 4] = reinterpret_cast<void *>(&native_item_decrement);
    table[0x2A4 / 4] = reinterpret_cast<void *>(&native_item_quantity);
    table[0x2A8 / 4] = reinterpret_cast<void *>(&native_item_has_quantity);
    table[0x2AC / 4] = reinterpret_cast<void *>(&native_item_get_quantity);
    table[0x2B0 / 4] = reinterpret_cast<void *>(&native_item_give);
    table[0x2B4 / 4] = reinterpret_cast<void *>(&native_item_preload);
    table[0x2B8 / 4] = reinterpret_cast<void *>(&native_item_apply);
    table[0x2BC / 4] = reinterpret_cast<void *>(&native_item_health);
    table[0x2C0 / 4] = reinterpret_cast<void *>(&native_item_armor);
}
