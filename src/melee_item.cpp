#include "melee_item.h"

#include "common.h"
#include "memory.h"
#include "motion_effect_struct.h"
#include "mstring.h"
#include "oldmath_po.h"
#include "vtbl.h"
#include "wds.h"

#include <algorithm>
#include <array>
#include <new>

VALIDATE_SIZE(melee_item, 0x14C);
VALIDATE_OFFSET(melee_item, first_trail_marker, 0x11C);
VALIDATE_OFFSET(melee_item, trail_samples, 0x144);

namespace {
void *__fastcall melee_delete(melee_item *self, void *, unsigned flags)
{
    self->~melee_item();
    if (flags & 1)
        mem_dealloc(self, sizeof(melee_item));
    return self;
}
void __fastcall melee_release(melee_item *self, void *)
{
    self->release_mem();
}
bool __fastcall melee_chunk(melee_item *, void *, void *, void *)
{
    return false;
}
bool __fastcall melee_query(melee_item *, void *)
{
    return true;
}
void __fastcall melee_unmash(melee_item *self, void *, generic_mash_header *header, void *object,
                             generic_mash_data_ptrs *data)
{
    self->un_mash(header, object, data);
}
void __fastcall melee_holster(melee_item *self, void *, bool visible)
{
    self->holster(visible);
}

void __fastcall melee_init_defaults(melee_item *, void *) {}
void __fastcall melee_enable_trail(melee_item *self, void *, bool enabled)
{
    self->enable_motion_trail(enabled);
}
void __fastcall melee_stop_trail(melee_item *self, void *)
{
    self->stop_motion_trail();
}
}  // namespace

void *melee_item::native_vtable(void **handheld_table)
{
    static std::array<void *, 0x300 / 4> table;
    std::copy_n(handheld_table, 0x2F8 / 4, table.begin());
    table[0] = reinterpret_cast<void *>(&melee_delete);
    table[0x10 / 4] = reinterpret_cast<void *>(&melee_release);
    table[0x20 / 4] = reinterpret_cast<void *>(&melee_chunk);
    table[0xE8 / 4] = reinterpret_cast<void *>(&melee_query);
    table[0x164 / 4] = reinterpret_cast<void *>(&melee_unmash);
    table[0x2C4 / 4] = reinterpret_cast<void *>(&melee_holster);
    table[0x2D4 / 4] = reinterpret_cast<void *>(&melee_init_defaults);
    table[0x2F8 / 4] = reinterpret_cast<void *>(&melee_enable_trail);
    table[0x2FC / 4] = reinterpret_cast<void *>(&melee_stop_trail);
    return table.data();
}

void melee_item::release_mem()
{
    for (auto *handle : {&first_trail_marker, &second_trail_marker}) {
        if (auto *value = handle->get_volatile_ptr()) {
            g_world_ptr->ent_mgr.destroy_entity(value);
            handle->field_0 = INVALID_HANDLE;
        }
    }
    handheld_item::release_mem();
}

void melee_item::un_mash(generic_mash_header *header, void *object, generic_mash_data_ptrs *data)
{
    handheld_item::un_mash(header, object, data);
    first_trail_marker.field_0 = INVALID_HANDLE;
    second_trail_marker.field_0 = INVALID_HANDLE;
}

void melee_item::holster(bool visible)
{
    auto stop = reinterpret_cast<void(__fastcall *)(melee_item *, void *)>(get_vfunc(m_vtbl, 0x2FC));
    stop(this, nullptr);
    handheld_item::holster(visible);
}

void melee_item::enable_motion_trail(bool enabled)
{
    auto *visual = field_104.get_volatile_ptr();
    if (!trail_enabled || !enabled || !visual) {
        auto stop = reinterpret_cast<void(__fastcall *)(melee_item *, void *)>(get_vfunc(m_vtbl, 0x2FC));
        stop(this, nullptr);
        return;
    }
    const auto make_marker = [visual](vhandle_type<marker> &handle, const vector3d &position) {
        if (!handle.get_volatile_ptr()) {
            auto *value = ::new (mem_alloc(sizeof(marker))) marker(make_unique_entity_id(), 0);
            value->set_abs_po(po_identity_matrix);
            value->set_abs_position(position);
            value->set_parent(visual);
            g_world_ptr->ent_mgr.add_dynamic_instanced_entity(value);
            handle.field_0 = value->my_handle;
        }
    };
    make_marker(first_trail_marker, first_trail_position);
    make_marker(second_trail_marker, second_trail_position);
    auto *first = first_trail_marker.get_volatile_ptr();
    if (first->field_18 && first->field_18->trail_active)
        return;
    auto *second = second_trail_marker.get_volatile_ptr();
    first->compute_sector(g_world_ptr->the_terrain, false, nullptr);
    if (!first->field_18)
        first->field_18 = new motion_effect_struct(first->my_handle, mString{""});
    auto &effect = *first->field_18;
    effect.trail_active = true;
    effect.draining_trail = false;

    if (!effect.trail) {
        effect.trail = new motion_trail_info{};
        effect.trail->field_0 = trail_samples;
        effect.trail->samples = new motion_trail_sample[trail_samples];
    }
    auto &trail = *effect.trail;
    trail.first = first;
    trail.second = second;
    trail.additive = additive_trail;
    trail.first_color = first_trail_color;
    trail.second_color = second_trail_color;
    trail.alpha = static_cast<uint8_t>((static_cast<unsigned>(first_trail_color[3]) + second_trail_color[3]) / 2);
    trail.remaining = 0.05f;
    trail.interval = 0.05f;
    trail.sample_count = 0;
    trail.next_sample = 0;
    trail.single_entity = false;
    trail.capacity = trail_samples;
    trail.samples[0].first = first->get_abs_position();
    trail.samples[0].second = second->get_abs_position();
    effect.remove_from_list();
}

void melee_item::stop_motion_trail()
{
    auto *first = first_trail_marker.get_volatile_ptr();
    if (!first || !first->field_18 || !first->field_18->trail_active)
        return;
    auto &effect = *first->field_18;
    effect.trail_active = false;
    effect.draining_trail = false;
    if (effect.trail) {
        delete[] effect.trail->samples;
        delete effect.trail;
        effect.trail = nullptr;
    }
    effect.remove_from_list();
}
