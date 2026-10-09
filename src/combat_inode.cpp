#include "combat_inode.h"

#include "ai_std_combat_target.h"
#include "ai_pedestrian.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "controller_inode.h"
#include "damage_inode.h"
#include "wds.h"
#include "combo_system.h"
#include "core_ai_resource.h"
#include "ai_tentacle_info.h"
#include "pendulum.h"
#include "physical_interface.h"
#include "rbc_def_distance.h"
#include "rigid_body.h"
#include "slab_allocator.h"
#include "polytube.h"
#include "polytubecustommaterial.h"
#include "variables.h"
#include <new>
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "oldmath_po.h"
#include "weapon_inode.h"
#include "handheld_item.h"
#include "damage_interface.h"
#include "event_manager.h"
#include "event.h"
#include "layer_state_machine.h"
#include "als_animation_logic_system.h"
#include "param_list.h"
#include "line_info.h"
#include "venom_inode.h"
#include "retaliation_inode.h"
#include "trigger.h"
#include "script.h"

#include <vtbl.h>

namespace {
struct combat_tether : ai_tentacle_info {
    vector3d previous_pivot;

    explicit combat_tether(const vector3d &pivot) : ai_tentacle_info(nullptr), previous_pivot(pivot)
    {
        field_A8 |= 1;
        init_positions(true);
        set_code_blend(1.0f, 0.0f);
        field_60 = pivot;
        end_pos = ZEROVEC;
        create_line(ZEROVEC, nullptr);
        if (tentacle != nullptr) {
            tentacle->the_spline.build(render_info->spline_flags, static_cast<spline::eSplineType>(3));
            tentacle->the_spline.rebuild_helper();
        }
        field_A8 &= ~4;
        base_node = end_node = nullptr;
        positions.m_size = tween_positions.m_size = 2;
        positions.m_data = new vector3d[2];
        tween_positions.m_data = new vector3d[2];
        render_info->num_sides = 2;
        render_info->radius = 0.1f;
        render_info->texture_scale = 1.5f;
        render_info->spline_flags = 2;
        create_tentacle(nullptr);
        if (webline_texture != nullptr) {
            tentacle->set_material(webline_texture);
            tentacle->field_D0->m_blend_mode = static_cast<nglBlendModeType>(2);
        }
    }
};
}  // namespace

namespace ai {

VALIDATE_SIZE(combat_inode, 0x328);
VALIDATE_SIZE(ped_combat_inode, 0x328);
VALIDATE_OFFSET(combat_inode, field_68, 0x68);

VALIDATE_SIZE(combat_inode::incoming_move, 0x94);

namespace {
void __fastcall combat_destruct(combat_inode *self, void *)
{
    self->_destruct_mashed_class();
}
void __fastcall combat_unmash(combat_inode *self, void *, mash_info_struct *info, void *context)
{
    self->_unmash(info, context);
}
void *__fastcall combat_delete(combat_inode *self, void *, unsigned flags)
{
    self->~combat_inode();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall combat_type(combat_inode *, void *)
{
    return 342;
}
bool __fastcall combat_subclass(combat_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall combat_true(combat_inode *, void *)
{
    return true;
}
void __fastcall combat_advance(combat_inode *self, void *, Float delta)
{
    self->_frame_advance(delta);
}
void __fastcall combat_activate(combat_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall combat_deactivate(combat_inode *self, void *)
{
    self->_deactivate();
}
int __fastcall combat_size(combat_inode *, void *)
{
    return sizeof(combat_inode);
}
void __fastcall combat_set_po(combat_inode *self, void *, const po &transform)
{
    if (self->field_82)
        entity_set_abs_po(self->field_C, transform);
}
int __fastcall combat_avoid_index(combat_inode *self, void *)
{
    return self->get_avoid_index();
}
int __fastcall combat_react_index(combat_inode *self, void *)
{
    return self->get_react_index();
}
bool __fastcall combat_select(combat_inode *self, void *, vhandle_type<actor> target, const vector3d &direction,
                              unsigned input, string_hash category, float eta, bool known, vhandle_type<actor> current)
{
    return self->select_satisfactory_move(target, direction, input, category, eta, known, current);
}
int __fastcall combat_immediate(combat_inode *self, void *)
{
    return self->get_immediate_index();
}
float __fastcall combat_eta(combat_inode *self, void *)
{
    using query_fn = float(__fastcall *)(als_inode *, void *, int);
    return reinterpret_cast<query_fn>(get_vfunc(self->field_28->m_vtbl, 0x38))(self->field_28, nullptr, 0);
}
void __fastcall combat_damage(combat_inode *self, void *, combat_inode::incoming_move move, const vector3d &direction)
{
    self->apply_move_damage(move, direction);
}
void __fastcall combat_responses(combat_inode *self, void *)
{
    self->consider_incoming_move_forced_responses();
}
int __fastcall combat_status(combat_inode *self, void *)
{
    return self->field_7C;
}
bool __fastcall combat_false(combat_inode *, void *)
{
    return false;
}
int __fastcall combat_zero(combat_inode *, void *)
{
    return 0;
}
void __fastcall combat_empty(combat_inode *, void *) {}
void __fastcall combat_empty_four(combat_inode *, void *, int, int, int, int) {}
void __fastcall combat_empty_one(combat_inode *, void *, int) {}
float __fastcall combat_combo_mod(combat_inode *self, void *)
{
    return self->field_54;
}
float __fastcall combat_meter(combat_inode *self, void *)
{
    return self->field_58;
}
float __fastcall combat_meter_max(combat_inode *self, void *)
{
    return self->field_60;
}
void __fastcall combat_alter_meter(combat_inode *self, void *, float amount)
{
    self->field_58 += amount;
    if (self->field_58 > self->field_60)
        self->field_58 = self->field_60;
    if (self->field_58 < self->field_5C)
        self->field_58 = self->field_5C;
}
void __fastcall combat_zero_meter(combat_inode *self, void *)
{
    self->field_58 = 0.0f;
    combat_alter_meter(self, nullptr, 0.0f);
}
void __fastcall combat_air_attack(combat_inode *self, void *)
{
    ++self->field_68;
}
void __fastcall combat_left_air(combat_inode *self, void *)
{
    self->left_air();
}
bool __fastcall combat_can_air(combat_inode *self, void *)
{
    return self->can_air_attack();
}
bool __fastcall combat_has_air(combat_inode *self, void *)
{
    return self->has_accum_air_attack();
}
bool __fastcall combat_wall(combat_inode *self, void *, entity_base *target)
{
    return self->find_attack_wall(target);
}
vector3d *__fastcall combat_wall_position(combat_inode *self, void *)
{
    return reinterpret_cast<vector3d *>(&self->field_B8);
}
vector3d *__fastcall combat_wall_normal(combat_inode *self, void *)
{
    return reinterpret_cast<vector3d *>(&self->field_C4);
}
bool __fastcall combat_has_wall(combat_inode *self, void *)
{
    return self->field_D0;
}
bool __fastcall combat_check_next(combat_inode *self, void *)
{
    return self->check_for_and_set_next_move();
}
bool __fastcall combat_chain(combat_inode *, void *, const void *, const combo_system_move *)
{
    return true;
}
bool __fastcall combat_has_cur(combat_inode *self, void *)
{
    return self->has_cur_move();
}
combo_system_move *__fastcall combat_cur(combat_inode *self, void *)
{
    return self->get_cur_move();
}
void __fastcall combat_clear_cur(combat_inode *self, void *)
{
    self->_clear_cur_move();
}
void __fastcall combat_end_cur(combat_inode *self, void *)
{
    self->end_cur_move();
}
bool __fastcall combat_has_next(combat_inode *self, void *)
{
    return self->has_next_move();
}
combo_system_move *__fastcall combat_next(combat_inode *self, void *)
{
    return self->get_next_move();
}
void __fastcall combat_clear_next(combat_inode *self, void *)
{
    self->_clear_next_move();
}
int __fastcall combat_attack_id(combat_inode *self, void *)
{
    return self->field_48;
}
bool __fastcall combat_performing(combat_inode *self, void *)
{
    return self->performing_combat();
}
void __fastcall combat_advance_move(combat_inode *self, void *)
{
    self->advance_to_next_move();
}
combat_inode::incoming_move *__fastcall combat_incoming(combat_inode *self, void *, int index)
{
    return &self->field_D8[index];
}
void __fastcall combat_update_pending(combat_inode *self, void *, combat_inode::incoming_move move)
{
    self->update_pending_move(move);
}
void __fastcall combat_receive(combat_inode *self, void *, combat_inode::incoming_move move, bool force)
{
    self->receive_and_act_on_results(move, force);
}
bool __fastcall combat_needs_react(combat_inode *self, void *, Float delta)
{
    return self->needs_hit_react(delta);
}
bool __fastcall combat_needs_avoid(combat_inode *self, void *, Float delta)
{
    return self->needs_hit_avoid(delta);
}
bool __fastcall combat_check_avoid(combat_inode *self, void *, string_hash category)
{
    return self->check_avoid_category(category);
}
int __fastcall combat_find_incoming(combat_inode *self, void *, string_hash category)
{
    return self->find_incoming_move(category);
}
string_hash *__fastcall combat_category_from_attack(combat_inode *, void *, string_hash *out, string_hash category,
                                                    string_hash, int)
{
    *out = category;
    return out;
}
string_hash *__fastcall combat_react_category(combat_inode *self, void *, string_hash *out)
{
    if (self->field_88 != -1) {
        *out = string_hash{self->field_90};
    } else {
        using index_fn = int(__fastcall *)(combat_inode *);
        using category_fn =
            string_hash *(__fastcall *)(combat_inode *, void *, string_hash *, string_hash, string_hash, int);
        const auto &move =
            self->field_D8[std::max(0, reinterpret_cast<index_fn>(get_vfunc(self->m_vtbl, 0x38))(self))].field_14;
        reinterpret_cast<category_fn>(get_vfunc(self->m_vtbl, 0xF4))(
            self, nullptr, out, move.field_8, move.field_4, move.field_24);
    }
    return out;
}
string_hash *__fastcall combat_avoid_category(combat_inode *self, void *, string_hash *out)
{
    if (self->field_8C != -1) {
        *out = string_hash{self->field_94};
    } else {
        using index_fn = int(__fastcall *)(combat_inode *);
        using category_fn =
            string_hash *(__fastcall *)(combat_inode *, void *, string_hash *, string_hash, string_hash, int);
        const auto &move =
            self->field_D8[std::max(0, reinterpret_cast<index_fn>(get_vfunc(self->m_vtbl, 0x34))(self))].field_14;
        reinterpret_cast<category_fn>(get_vfunc(self->m_vtbl, 0xFC))(
            self, nullptr, out, move.field_C, move.field_4, move.field_24);
    }
    return out;
}
void __fastcall combat_avoid(combat_inode *self, void *)
{
    self->avoid_attack();
}
void __fastcall combat_avoid_all(combat_inode *self, void *, vhandle_type<entity> source)
{
    self->avoid_all_attacks(source);
}
void __fastcall combat_clear_target(combat_inode *self, void *, vhandle_type<actor> target)
{
    self->clear_from_target(target);
}
void __fastcall combat_clear_targets(combat_inode *self, void *)
{
    self->field_1C = self->field_20 = 0;
}
vector3d *__fastcall combat_direction(combat_inode *self, void *, vector3d *out)
{
    *out = self->get_attack_direction();
    return out;
}
float __fastcall combat_zero_float(combat_inode *, void *)
{
    return 0.0f;
}
bool __fastcall combat_track(combat_inode *self, void *)
{
    return self->field_74 && reinterpret_cast<const int *>(self->field_74)[6] == 3;
}
void __fastcall combat_non_combo(combat_inode *self, void *, string_hash react, string_hash attack, string_hash avoid,
                                 int type, vhandle_type<entity> source, const vector3d &direction, bool force)
{
    self->possible_non_combo_system_attack(react, attack, avoid, type, source, direction, force);
}
bool __fastcall combat_forced(combat_inode *self, void *, string_hash react, string_hash attack, string_hash avoid,
                              int type, vhandle_type<entity> source, const vector3d &direction, bool pending,
                              bool force)
{
    return self->consider_forced_responses(react, attack, avoid, type, source, direction, pending, force);
}
}  // namespace

void *combat_inode::native_vtable()
{
    auto **base = static_cast<void **>(info_node::native_vtable());

    static void *table[] = {
        reinterpret_cast<void *>(&combat_destruct),
        reinterpret_cast<void *>(&combat_unmash),
        reinterpret_cast<void *>(&combat_delete),
        reinterpret_cast<void *>(&combat_type),
        reinterpret_cast<void *>(&combat_subclass),
        base[5],
        reinterpret_cast<void *>(&combat_true),
        reinterpret_cast<void *>(&combat_advance),
        reinterpret_cast<void *>(&combat_activate),
        reinterpret_cast<void *>(&combat_deactivate),
        base[10],
        reinterpret_cast<void *>(&combat_size),
        reinterpret_cast<void *>(&combat_set_po),
        reinterpret_cast<void *>(&combat_avoid_index),
        reinterpret_cast<void *>(&combat_react_index),
        reinterpret_cast<void *>(&combat_select),
        reinterpret_cast<void *>(&combat_immediate),
        reinterpret_cast<void *>(&combat_eta),
        reinterpret_cast<void *>(&combat_damage),
        reinterpret_cast<void *>(&combat_responses),
        reinterpret_cast<void *>(&combat_status),
        reinterpret_cast<void *>(&combat_false),
        reinterpret_cast<void *>(&combat_zero),
        reinterpret_cast<void *>(&combat_empty),
        reinterpret_cast<void *>(&combat_empty_four),
        reinterpret_cast<void *>(&combat_empty_one),
        reinterpret_cast<void *>(&combat_combo_mod),
        reinterpret_cast<void *>(&combat_meter),
        reinterpret_cast<void *>(&combat_meter_max),
        reinterpret_cast<void *>(&combat_alter_meter),
        reinterpret_cast<void *>(&combat_zero_meter),
        reinterpret_cast<void *>(&combat_air_attack),
        reinterpret_cast<void *>(&combat_left_air),
        reinterpret_cast<void *>(&combat_can_air),
        reinterpret_cast<void *>(&combat_has_air),
        reinterpret_cast<void *>(&combat_wall),
        reinterpret_cast<void *>(&combat_wall_position),
        reinterpret_cast<void *>(&combat_wall_normal),
        reinterpret_cast<void *>(&combat_has_wall),
        reinterpret_cast<void *>(&combat_check_next),
        reinterpret_cast<void *>(&combat_chain),
        reinterpret_cast<void *>(&combat_has_cur),
        reinterpret_cast<void *>(&combat_cur),
        reinterpret_cast<void *>(&combat_clear_cur),
        reinterpret_cast<void *>(&combat_end_cur),
        reinterpret_cast<void *>(&combat_has_next),
        reinterpret_cast<void *>(&combat_next),
        reinterpret_cast<void *>(&combat_clear_next),
        reinterpret_cast<void *>(&combat_attack_id),
        reinterpret_cast<void *>(&combat_performing),
        reinterpret_cast<void *>(&combat_advance_move),
        reinterpret_cast<void *>(&combat_incoming),
        reinterpret_cast<void *>(&combat_false),
        reinterpret_cast<void *>(&combat_false),
        reinterpret_cast<void *>(&combat_update_pending),
        reinterpret_cast<void *>(&combat_receive),
        reinterpret_cast<void *>(&combat_needs_react),
        reinterpret_cast<void *>(&combat_needs_avoid),
        reinterpret_cast<void *>(&combat_check_avoid),
        reinterpret_cast<void *>(&combat_find_incoming),
        reinterpret_cast<void *>(&combat_react_category),
        reinterpret_cast<void *>(&combat_category_from_attack),
        reinterpret_cast<void *>(&combat_avoid_category),
        reinterpret_cast<void *>(&combat_category_from_attack),
        reinterpret_cast<void *>(&combat_avoid),
        reinterpret_cast<void *>(&combat_avoid_all),
        reinterpret_cast<void *>(&combat_clear_target),
        reinterpret_cast<void *>(&combat_clear_targets),
        reinterpret_cast<void *>(&combat_direction),
        reinterpret_cast<void *>(&combat_zero_float),
        reinterpret_cast<void *>(&combat_empty_one),
        reinterpret_cast<void *>(&combat_zero),
        reinterpret_cast<void *>(&combat_track),
        reinterpret_cast<void *>(&combat_non_combo),
        reinterpret_cast<void *>(&combat_forced),
        reinterpret_cast<void *>(&combat_empty_one),
    };
    return table;
}

namespace {
void *__fastcall ped_combat_delete(ped_combat_inode *self, void *, unsigned flags)
{
    self->~ped_combat_inode();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall ped_combat_type(ped_combat_inode *, void *)
{
    return 158;
}
bool __fastcall ped_combat_subclass(ped_combat_inode *, void *, unsigned type)
{
    return type == 342 || type == 537 || type == 573;
}
void __fastcall ped_combat_receive(ped_combat_inode *self, void *, combat_inode::incoming_move move, bool force)
{
    self->receive_and_act_on_results(move, force);
}
bool __fastcall ped_combat_forced(ped_combat_inode *self, void *, string_hash reaction, string_hash attack,
                                  string_hash avoid, int type, vhandle_type<entity> source, const vector3d &direction,
                                  bool pending, bool force)
{
    return self->consider_forced_responses(reaction, attack, avoid, type, source, direction, pending, force);
}
}  // namespace

void *ped_combat_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 76> result;
        std::copy_n(static_cast<void **>(combat_inode::native_vtable()), result.size(), result.data());
        result[2] = reinterpret_cast<void *>(&ped_combat_delete);
        result[3] = reinterpret_cast<void *>(&ped_combat_type);
        result[4] = reinterpret_cast<void *>(&ped_combat_subclass);
        result[0xDC / 4] = reinterpret_cast<void *>(&ped_combat_receive);
        result[0x128 / 4] = reinterpret_cast<void *>(&ped_combat_forced);
        return result;
    }();
    return table.data();
}

ped_combat_inode::ped_combat_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[158]);
}

ped_combat_inode::ped_combat_inode(from_mash_in_place_constructor *tag) : combat_inode(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[158]);
}

void ped_combat_inode::receive_and_act_on_results(incoming_move &move, bool)
{
    field_8->field_64->get_abs_po();
    auto *source = vhandle_type<actor>{entity_base_vhandle{static_cast<unsigned>(move.field_4)}}.get_volatile_ptr();
    if (source) {
        source->get_abs_po();
        field_8->field_64->get_abs_po();
        source->is_an_actor();
    }
    if (move.field_14.field_28 == 2) {
        field_D4 = move.field_14.field_4.source_hash_code;
        auto *pedestrian = static_cast<pedestrian_inode *>(field_8->get_info_node(pedestrian_inode::default_id, true));
        pedestrian->m_hit_points -= bit_cast<float>(move.field_14.field_20);
        if (pedestrian->m_hit_points <= 0.0f) {
            pedestrian->m_hit_points = 0.0f;
            pedestrian->field_D1 = true;
        }
        if (source) {
            source->damage_ifc();
            if (pedestrian->field_D1) {
                auto *combat = static_cast<combat_inode *>(source->get_ai_core()->get_info_node(default_id, true));
                using clear_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<actor>);
                reinterpret_cast<clear_fn>(get_vfunc(combat->m_vtbl, 0x108))(
                    combat, nullptr, vhandle_type<actor>{field_C->my_handle});
            }
        }
        event_manager::raise_event(event::FED_UPON, field_C->my_handle);
    }
    using pending_fn = void(__fastcall *)(combat_inode *, void *, incoming_move);
    reinterpret_cast<pending_fn>(get_vfunc(m_vtbl, 0xD8))(this, nullptr, move);
}

bool ped_combat_inode::consider_forced_responses(string_hash reaction, string_hash attack, string_hash avoid, int type,
                                                 vhandle_type<entity> source, const vector3d &direction, bool pending,
                                                 bool force)
{
    const bool accepted =
        combat_inode::consider_forced_responses(reaction, attack, avoid, type, source, direction, pending, force);
    if (accepted) {
        using avoid_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<entity>);
        reinterpret_cast<avoid_fn>(get_vfunc(m_vtbl, 0x104))(this, nullptr, source);
        clear_forced_avoid_needed();
    }
    return accepted;
}

namespace {
void *__fastcall venom_combat_delete(venom_combat_inode *self, void *, unsigned flags)
{
    self->~venom_combat_inode();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall venom_combat_type(venom_combat_inode *, void *)
{
    return 435;
}
bool __fastcall venom_combat_subclass(venom_combat_inode *, void *, unsigned type)
{
    return type == 342 || type == 537 || type == 573;
}
int __fastcall venom_combat_size(venom_combat_inode *, void *)
{
    return sizeof(venom_combat_inode);
}
void __fastcall venom_combat_activate(venom_combat_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
bool __fastcall venom_combat_needs_react(venom_combat_inode *self, void *, Float elapsed)
{
    return self->needs_hit_react(elapsed);
}
bool __fastcall venom_combat_forced(venom_combat_inode *self, void *, string_hash reaction, string_hash attack,
                                    string_hash avoid, int type, vhandle_type<entity> source, const vector3d &direction,
                                    bool pending, bool force)
{
    return self->consider_forced_responses(reaction, attack, avoid, type, source, direction, pending, force);
}

}  // namespace

VALIDATE_SIZE(venom_combat_inode, 0x32C);
VALIDATE_OFFSET(venom_combat_inode, allow_hit_react, 0x328);

void *venom_combat_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 76> result;
        std::copy_n(static_cast<void **>(combat_inode::native_vtable()), result.size(), result.data());
        result[2] = reinterpret_cast<void *>(&venom_combat_delete);
        result[3] = reinterpret_cast<void *>(&venom_combat_type);
        result[4] = reinterpret_cast<void *>(&venom_combat_subclass);
        result[8] = reinterpret_cast<void *>(&venom_combat_activate);
        result[11] = reinterpret_cast<void *>(&venom_combat_size);
        result[56] = reinterpret_cast<void *>(&venom_combat_needs_react);
        result[74] = reinterpret_cast<void *>(&venom_combat_forced);
        return result;
    }();
    return table.data();
}


venom_combat_inode::venom_combat_inode(from_mash_in_place_constructor *tag) : combat_inode(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[435]);
}


void venom_combat_inode::_activate(ai_core *core)
{
    combat_inode::_activate(core);
    allow_hit_react = false;
}


bool venom_combat_inode::needs_hit_react(Float elapsed)
{
    auto *venom =
        static_cast<::venom_inode *>(field_8->get_info_node(string_hash{static_cast<int>(to_hash("venom"))}, true));
    return (allow_hit_react || field_82) && !venom->field_24 && combat_inode::needs_hit_react(elapsed);
}


bool venom_combat_inode::consider_forced_responses(string_hash reaction, string_hash attack, string_hash avoid,
                                                   int type, vhandle_type<entity> source, const vector3d &direction,
                                                   bool pending, bool force)
{
    const bool accepted =
        combat_inode::consider_forced_responses(reaction, attack, avoid, type, source, direction, pending, force);
    auto *venom =
        static_cast<::venom_inode *>(field_8->get_info_node(string_hash{static_cast<int>(to_hash("venom"))}, true));
    if (!accepted && !venom->field_24 && venom->accepts_knockdown_attack(attack)) {
        if (get_react_index() == -1)
            return false;
        auto *combat = static_cast<venom_combat_inode *>(field_8->get_info_node(combat_inode::default_id, true));
        combat->allow_hit_react = false;
        venom->field_24 = true;
        vector3d knockdown_direction = direction;
        if (venom->field_B8 != nullptr) {
            const vector3d towards_spark = venom->field_B8->get_position() - field_C->get_abs_position();
            if (std::fabs(dot(towards_spark, direction)) < 1.0f)
                knockdown_direction = towards_spark;
        }
        try_set_forced_react_needed(string_hash{static_cast<int>(to_hash("KnockDownSmall"))},
                                    attack,
                                    type,
                                    source,
                                    knockdown_direction,
                                    pending,
                                    true);
        find_func_and_spawn_new_thread(field_C, string_hash{static_cast<int>(to_hash("you_got_knocked_out()"))});
        script::exec_thread(false);
        return accepted;
    }
    if (!accepted && !venom->field_24) {
        const auto *retaliation = static_cast<const retaliation_inode *>(
            field_8->get_info_node(string_hash{static_cast<int>(to_hash("RETALIATION"))}, false));
        if (retaliation == nullptr || !retaliation->calc_damage_since_last_retaliation(0))
            return accepted;
    }
    using clear_fn = void(__fastcall *)(combat_inode *, void *);
    using avoid_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<entity>);
    reinterpret_cast<clear_fn>(get_vfunc(m_vtbl, 0x100))(this, nullptr);
    reinterpret_cast<avoid_fn>(get_vfunc(m_vtbl, 0x104))(this, nullptr, source);
    clear_forced_avoid_needed();
    return true;
}

void combat_inode::_destruct_mashed_class()
{
    reinterpret_cast<string_hash &>(field_70).destruct_mashed_class();
    reinterpret_cast<string_hash &>(field_90).destruct_mashed_class();
    reinterpret_cast<string_hash &>(field_94).destruct_mashed_class();
    reinterpret_cast<string_hash &>(field_98).destruct_mashed_class();
    reinterpret_cast<string_hash &>(field_D4).destruct_mashed_class();
    using destruct_fn = void(__fastcall *)(incoming_move *);
    for (auto &move : field_D8)
        reinterpret_cast<destruct_fn>(get_vfunc(move.m_vtbl, 0))(&move);
    info_node::_destruct_mashed_class();
}

void combat_inode::_deactivate()
{
    if (field_34) {
        field_34->~ai_tentacle_info();
        if (slab_allocator::get_max_object_size() >= 0xE4)
            slab_allocator::deallocate(field_34, nullptr);
        else
            ::operator delete(field_34);
    }
}

int combat_inode::get_immediate_index() const
{
    int best = -1;
    for (int i = 0; i < 4; ++i)
        if (field_D8[i].field_10 && (best == -1 || field_D8[i].field_C < field_D8[best].field_C))
            best = i;
    return best;
}

bool combat_inode::check_avoid_category(string_hash category) const
{
    auto *combo = field_8->field_6C->field_10;
    if (!combo)
        return false;
    for (int i = 0; i < combo->field_3C.m_size; ++i)
        if (*combo->field_3C.m_data[static_cast<uint16_t>(i)] == category)
            return true;
    return false;
}

int combat_inode::find_incoming_move(string_hash category) const
{
    for (int i = 0; i < 4; ++i) {
        const auto &move = field_D8[i];
        if (move.field_10 && (move.field_14.field_8 == category || move.field_14.field_C == category))
            return i;
    }
    return -1;
}

bool combat_inode::needs_hit_avoid(Float)
{
    if ((!field_83 || field_86) && field_8C != -1)
        return true;
    using index_fn = int(__fastcall *)(combat_inode *);
    using check_fn = bool(__fastcall *)(combat_inode *, void *, string_hash);
    const int index = reinterpret_cast<index_fn>(get_vfunc(m_vtbl, 0x34))(this);
    return index >= 0 &&
           reinterpret_cast<check_fn>(get_vfunc(m_vtbl, 0xE8))(this, nullptr, field_D8[index].field_14.field_4);
}

void combat_inode::avoid_attack()
{
    using index_fn = int(__fastcall *)(combat_inode *);
    using clear_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<actor>);
    const int index = reinterpret_cast<index_fn>(get_vfunc(m_vtbl, 0x34))(this);
    if (index >= 0) {
        auto &move = field_D8[index];
        auto *attacker = entity_base_vhandle{static_cast<unsigned>(move.field_4)}.get_volatile_ptr();
        auto *combat = static_cast<combat_inode *>(attacker->get_ai_core()->get_info_node(default_id, true));
        reinterpret_cast<clear_fn>(get_vfunc(combat->m_vtbl, 0x108))(
            combat, nullptr, vhandle_type<actor>{field_8->field_64->my_handle});
        move.field_10 = 0;
    }
}

void combat_inode::avoid_all_attacks(vhandle_type<entity> source)
{
    using clear_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<actor>);
    const auto clear = [this](combat_inode *combat) {
        reinterpret_cast<clear_fn>(get_vfunc(combat->m_vtbl, 0x108))(
            combat, nullptr, vhandle_type<actor>{field_8->field_64->my_handle});
    };
    if (auto *attacker = source.get_volatile_ptr())
        if (auto *core = attacker->get_ai_core())
            if (auto *combat = static_cast<combat_inode *>(core->get_info_node(default_id, false)))
                clear(combat);
    for (auto &move : field_D8) {
        if (move.field_10) {
            if (auto *attacker = entity_base_vhandle{static_cast<unsigned>(move.field_4)}.get_volatile_ptr())
                clear(static_cast<combat_inode *>(attacker->get_ai_core()->get_info_node(default_id, true)));
            move.field_10 = 0;
        }
    }
}

void combat_inode::end_cur_move()
{
    using has_fn = bool(__fastcall *)(combat_inode *);
    using move_fn = combo_system_move *(__fastcall *)(combat_inode *);
    if (reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0xA4))(this)) {
        const auto *move = reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xA8))(this);
        const int effect = move->field_4.field_28;
        if ((effect >= 3 && effect <= 12) || effect == 16)
            disable_weapon_based_effect(move);
    }
}

bool combat_inode::performing_combat()
{
    using move_fn = combo_system_move *(__fastcall *)(combat_inode *);
    return field_28->get_category_id(static_cast<als::layer_types>(0)) ==
           reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xA8))(this)->field_4.field_4;
}

void combat_inode::advance_to_next_move()
{
    using action_fn = void(__fastcall *)(combat_inode *);
    reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0xB0))(this);
    static int attack_id_base = -1;
    field_48 = attack_id_base++;
    field_40 = field_44;
    field_1C = field_20;
    field_50 = 0;
    reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0xBC))(this);
    field_7C = 3;
}

void combat_inode::update_pending_move(const incoming_move &move)
{
    incoming_move *slot = nullptr;
    for (auto &current : field_D8)
        if (current.field_10 && current.field_4 == move.field_4) {
            slot = &current;
            break;
        }
    if (!slot)
        for (auto &current : field_D8)
            if (!current.field_10) {
                slot = &current;
                break;
            }
    if (!slot)
        for (auto &current : field_D8)
            if (static_cast<unsigned>(current.field_8 + 2) < static_cast<unsigned>(g_world_ptr->time_manager.field_C)) {
                slot = &current;
                break;
            }
    if (slot) {
        slot->field_8 = move.field_8;
        slot->field_C = move.field_C;
        slot->field_4 = move.field_4;
        slot->field_10 = move.field_10;
        slot->field_14 = move.field_14;
        slot->field_90 = move.field_90;
    }
}

void combat_inode::try_set_forced_react_needed(string_hash reaction, string_hash attack, int type,
                                               vhandle_type<entity> source, vector3d direction, bool pending,
                                               bool force)
{
    if (reaction == string_hash{0} || (!force && field_82))
        return;
    field_90 = reaction.source_hash_code;
    field_88 = g_world_ptr->time_manager.field_C;
    field_85 = field_82;
    if (direction.length2() < EPSILON)
        if (auto *attacker = source.get_volatile_ptr()) {
            direction = field_C->get_abs_position() - attacker->get_abs_position();
            direction.normalize();
        }
    if (field_8C == -1) {
        field_98 = attack.source_hash_code;
        field_9C = type;
        field_A0 = source.field_0.field_0;
        *reinterpret_cast<vector3d *>(&field_A4) = direction;
        field_B0 = pending;
    }
}

void combat_inode::try_set_forced_avoid_needed(string_hash reaction, string_hash attack, int type,
                                               vhandle_type<entity> source, vector3d direction, bool pending,
                                               bool force)
{
    if (reaction == string_hash{0} || (!force && field_83))
        return;
    field_94 = reaction.source_hash_code;
    field_8C = g_world_ptr->time_manager.field_C;
    field_86 = field_83;
    if (direction.length2() < EPSILON)
        if (auto *attacker = source.get_volatile_ptr()) {
            direction = field_C->get_abs_position() - attacker->get_abs_position();
            direction.normalize();
        }
    field_98 = attack.source_hash_code;
    field_9C = type;
    field_A0 = source.field_0.field_0;
    *reinterpret_cast<vector3d *>(&field_A4) = direction;
    field_B0 = pending;
}

void combat_inode::apply_move_damage(const incoming_move &move, const vector3d &direction)
{
    auto *source = entity_base_vhandle{static_cast<unsigned>(move.field_4)}.get_volatile_ptr();
    actor *attacker = nullptr;
    if (source) {
        if (source->is_an_actor())
            attacker = static_cast<actor *>(source);
    }
    field_D4 = move.field_14.field_4.source_hash_code;
    field_30->apply_damage(attacker,
                           static_cast<int>(bit_cast<float>(move.field_14.field_20)),
                           direction,
                           move.field_14.field_8,
                           move.field_14.field_4,
                           move.field_14.field_C,
                           false,
                           move.field_14.field_24,
                           true);
    if (source) {
        if (auto *core = attacker->get_ai_core()) {
            if (core->field_50.get_optional_pb_int(string_hash{"always_apply_subdue"}, 0, nullptr))
                field_30->set_subdue(attacker, 1.0f, direction);
        }
    }
}

bool combat_inode::consider_forced_responses(string_hash reaction, string_hash attack, string_hash avoid, int type,
                                             vhandle_type<entity> source, const vector3d &direction, bool pending,
                                             bool force)
{
    using action_fn = void(__fastcall *)(combat_inode *);
    using check_fn = bool(__fastcall *)(combat_inode *, void *, string_hash);
    using avoid_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<entity>);
    if (field_81) {
        clear_forced_react_needed();
        reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0x100))(this);
        return true;
    }
    if (reinterpret_cast<check_fn>(get_vfunc(m_vtbl, 0xE8))(this, nullptr, attack)) {
        try_set_forced_avoid_needed(avoid, attack, type, source, direction, pending, force);
        clear_forced_react_needed();
        return true;
    }
    const string_hash reject{"feed_reject_als_cat"};
    if (reaction == string_hash{"Feed_Loop_By_Venom"} && field_8->field_50.does_parameter_exist(reject)) {
        try_set_forced_avoid_needed(
            field_8->field_50.get_pb_hash(reject), attack, type, source, direction, pending, force);
        clear_forced_react_needed();
        reinterpret_cast<avoid_fn>(get_vfunc(m_vtbl, 0x104))(this, nullptr, source);
        if (auto *attacker = source.get_volatile_ptr()) {
            if (auto *core = attacker->get_ai_core()) {
                if (auto *damage = static_cast<damage_inode *>(core->get_info_node(damage_inode::default_id, false))) {
                    const float amount =
                        field_8->field_50.get_optional_pb_float(string_hash{"feed_burst_damage"}, 15.0f, nullptr);
                    damage->apply_forced_damage(static_cast<int>(amount),
                                                field_C->get_abs_position() - attacker->get_abs_position(),
                                                string_hash{"Electro_Eject"},
                                                false);
                }
            }
        }
        return true;
    }
    return false;
}

void combat_inode::possible_non_combo_system_attack(string_hash reaction, string_hash attack, string_hash avoid,
                                                    int type, vhandle_type<entity> source, const vector3d &direction,
                                                    bool force)
{
    using category_fn =
        string_hash *(__fastcall *)(combat_inode *, void *, string_hash *, string_hash, string_hash, int);
    using response_fn = bool(__fastcall *)(combat_inode *,
                                           void *,
                                           string_hash,
                                           string_hash,
                                           string_hash,
                                           int,
                                           vhandle_type<entity>,
                                           const vector3d &,
                                           bool,
                                           bool);
    string_hash resolved;
    reinterpret_cast<category_fn>(get_vfunc(m_vtbl, 0xF4))(this, nullptr, &resolved, reaction, attack, type);
    try_set_forced_react_needed(resolved, attack, type, source, direction, false, force);
    reinterpret_cast<response_fn>(get_vfunc(m_vtbl, 0x128))(
        this, nullptr, reaction, attack, avoid, type, source, direction, false, force);
}

vector3d combat_inode::get_attack_direction()
{
    vector3d direction{0.0f, 0.0f, 1.0f};
    if (field_88 == -1 && field_8C == -1) {
        using index_fn = int(__fastcall *)(combat_inode *);
        const int index = reinterpret_cast<index_fn>(get_vfunc(m_vtbl, 0x40))(this);
        if (index != -1) {
            if (auto *attacker =
                    entity_base_vhandle{static_cast<unsigned>(field_D8[index].field_4)}.get_volatile_ptr()) {
                direction = field_8->field_64->get_abs_position() - attacker->get_abs_position();
                direction.normalize();
            }
        }
    } else {
        const auto &forced_direction = *reinterpret_cast<const vector3d *>(&field_A4);
        entity_base_vhandle{static_cast<unsigned>(field_A0)}.get_volatile_ptr();
        if (forced_direction.length2() > EPSILON)
            direction = forced_direction;
    }
    return direction;
}

bool combat_inode::select_satisfactory_move(vhandle_type<actor> target, const vector3d &direction, unsigned input,
                                            string_hash category, float eta, bool target_known,
                                            vhandle_type<actor> current_target)
{
    using level_fn = int(__fastcall *)(combat_inode *);
    using has_fn = bool(__fastcall *)(combat_inode *);
    using wall_fn = bool(__fastcall *)(combat_inode *, void *, entity_base *);
    auto &moves = field_8->field_6C->field_10->field_0;
    int candidates[100];
    int count = 0;
    int best = 0;
    bool checked_wall = false;
    for (int index = 0; index < moves.m_size; ++index) {
        const auto *move = moves.m_data[index];
        const float level = static_cast<float>(reinterpret_cast<level_fn>(get_vfunc(m_vtbl, 0x11C))(this));
        int score = move->requirements_satisfaction(target, direction, input, category, eta, target_known, level);
        if (best <= score) {
            if (move->field_4.field_24 == 13) {
                if (!checked_wall) {
                    reinterpret_cast<wall_fn>(get_vfunc(m_vtbl, 0x8C))(
                        this, nullptr, current_target.get_volatile_ptr());
                    checked_wall = true;
                }
                if (!reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0x98))(this))
                    score -= 1000;
            }
            if (score > best) {
                best = score;
                candidates[0] = index;
                count = 1;
            } else if (score == best) {
                candidates[count++] = index;
            }
        }
    }
    if (count == 0 || reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0xB4))(this) || field_B4)
        return false;
    field_44 = candidates[static_cast<unsigned>(std::rand() * static_cast<double>(count) * (1.0f / 32768.0f))];
    field_20 = (moves.m_data[static_cast<uint16_t>(field_44)]->field_80.field_10.field_4 == 5 ? current_target : target)
                   .field_0.field_0;
    field_7C = 2;
    return true;
}

bool combat_inode::check_for_and_set_next_move()
{
    using controller_fn = bool(__fastcall *)(controller_inode *);
    using has_fn = bool(__fastcall *)(combat_inode *);
    using move_fn = combo_system_move *(__fastcall *)(combat_inode *);
    using target_fn = vhandle_type<actor> *(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor> *);
    using target_bool_fn = bool(__fastcall *)(base_full_target_inode *);
    using trigger_fn = unsigned(__fastcall *)(controller_inode *, void *, vector3d);
    using signal_fn = void(__fastcall *)(als_inode *, void *, Float &, string_hash &);
    using select_fn = bool(__fastcall *)(combat_inode *,
                                         void *,
                                         vhandle_type<actor>,
                                         const vector3d &,
                                         unsigned,
                                         string_hash,
                                         float,
                                         bool,
                                         vhandle_type<actor>);
    const bool pending = field_24 && reinterpret_cast<controller_fn>(get_vfunc(field_24->m_vtbl, 0x40))(field_24);
    if (!pending)
        return false;
    auto *layer = field_28->get_als_layer(static_cast<als::layer_types>(0));
    if (!layer->is_interruptable())
        return false;
    vhandle_type<actor> current;
    reinterpret_cast<target_fn>(get_vfunc(field_2C->m_vtbl, 0x64))(field_2C, nullptr, &current);
    bool known = current.get_volatile_ptr() != nullptr;
    vhandle_type<actor> target{0};
    bool retain_current = false;
    if (!field_80) {
        if (reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0xA4))(this) &&
            reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0xC4))(this) &&
            reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xA8))(this)->field_4.field_68) {
            target = {entity_base_vhandle{static_cast<unsigned>(field_1C)}};
            known = false;
            retain_current = true;
        }
    }
    if (!retain_current && (field_80 || reinterpret_cast<target_bool_fn>(get_vfunc(field_2C->m_vtbl, 0x6C))(field_2C) ||
                            g_world_ptr->get_hero_ptr(0) == field_C))
        reinterpret_cast<target_fn>(get_vfunc(field_2C->m_vtbl, 0x38))(field_2C, nullptr, &target);
    const vector3d stick = field_24->get_axis(static_cast<controller_inode::eControllerAxis>(2));
    auto *target_actor = target.get_volatile_ptr();
    layer->set_desired_param(als::param{52, target_actor ? 1.0f : 0.0f});
    const vector3d direction =
        target_actor ? target_actor->get_abs_position() - field_8->field_64->get_abs_position() : stick;
    const unsigned input = reinterpret_cast<trigger_fn>(get_vfunc(field_24->m_vtbl, 0x44))(field_24, nullptr, stick);
    const auto category = field_28->get_category_id(static_cast<als::layer_types>(0));
    string_hash signal_category;
    Float eta{0.0f};
    reinterpret_cast<signal_fn>(get_vfunc(field_28->m_vtbl, 0x34))(field_28, nullptr, eta, signal_category);
    eta = signal_category == category ? eta - g_world_ptr->time_manager.get_level_time() : -100.0f;
    using hero_fn = bool(__fastcall *)(actor *);
    if (reinterpret_cast<select_fn>(get_vfunc(m_vtbl, 0x3C))(
            this, nullptr, target, direction, input, category, eta, known, current) &&
        reinterpret_cast<hero_fn>(get_vfunc(field_C->m_vtbl, 0x4C))(field_C) &&
        reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0xA4))(this) &&
        reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xA8))(this)->field_80.field_10.field_4 != 4 &&
        reinterpret_cast<has_fn>(get_vfunc(m_vtbl, 0xB4))(this)) {
        const auto *next = reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xB8))(this);
        if (next->field_80.field_10.field_4 != 4 && next->field_4.field_28 != 1) {
            if (field_1C != field_20)
                field_54 += 3.0f;
            if (reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xA8))(this)->field_80.field_4.field_8 !=
                next->field_80.field_4.field_8)
                field_54 += 2.0f;
        }
    }
    return false;
}

void combat_inode::disable_weapon_based_effect(const combo_system_move *move)
{
    auto *weapons = static_cast<weapon_inode *>(field_8->get_info_node(weapon_inode::default_id, true));
    const int effect = move->field_4.field_28;
    const auto handle = weapons->get_weapon_handle(static_cast<uint16_t>(effect == 16 ? 0 : effect - 3));
    const auto stop = [](handheld_item *weapon) {
        using predicate_fn = bool(__fastcall *)(handheld_item *);
        using stop_fn = void(__fastcall *)(handheld_item *);
        if (weapon && reinterpret_cast<predicate_fn>(get_vfunc(weapon->m_vtbl, 0xE0))(weapon) &&
            (weapon->field_10C & 0x1000))
            reinterpret_cast<stop_fn>(get_vfunc(weapon->m_vtbl, 0x300))(weapon);
    };
    stop(handle.get_volatile_ptr());
    if (effect == 16) {
        weapons->get_weapon_handle(1);

        stop(handle.get_volatile_ptr());
    }
}

void combat_inode::send_weapon_attack(const incoming_move &move)
{
    auto *source = vhandle_type<actor>{entity_base_vhandle{static_cast<unsigned>(move.field_4)}}.get_volatile_ptr();
    if (!source)
        return;
    auto *weapons = static_cast<weapon_inode *>(source->get_ai_core()->get_info_node(weapon_inode::default_id, true));
    const int effect = move.field_14.field_28;
    const auto handle = weapons->get_weapon_handle(static_cast<uint16_t>(effect == 16 ? 0 : effect - 3));
    auto *source_combat = static_cast<combat_inode *>(source->get_ai_core()->get_info_node(default_id, true));
    source_combat->field_78 = handle.field_0.field_0;
    using attack_fn = void(__fastcall *)(
        handheld_item *, void *, entity_base_vhandle, entity_base_vhandle, const combo_system_move::results *);
    const auto attack = [&](handheld_item *weapon) {
        reinterpret_cast<attack_fn>(get_vfunc(weapon->m_vtbl, 0x2F4))(
            weapon, nullptr, source->my_handle, field_C->my_handle, &move.field_14);
    };
    attack(handle.get_volatile_ptr());
    if (effect == 16)
        attack(weapons->get_weapon_handle(1).get_volatile_ptr());
}

void combat_inode::receive_and_act_on_results(incoming_move &move, bool force)
{
    int effect = move.field_14.field_28;
    const bool weapon_effect = (effect >= 3 && effect <= 12) || effect == 16;
    if (weapon_effect && !force) {
        send_weapon_attack(move);
        return;
    }
    vector3d direction = -field_8->field_64->get_abs_po().get_z_facing();
    auto *source = vhandle_type<actor>{entity_base_vhandle{static_cast<unsigned>(move.field_4)}}.get_volatile_ptr();
    entity *attacker = nullptr;
    if (source) {
        direction = field_8->field_64->get_abs_position() - source->get_abs_position();
        if (source->is_an_actor())
            attacker = source;
    }
    direction.normalize();
    if (weapon_effect) {
        effect = move.field_14.field_28 = 0;
        move.field_C = 0.0f;
    }
    using damage_fn = void(__fastcall *)(combat_inode *, void *, incoming_move, const vector3d &);
    if (effect == 0 || effect == 2) {
        reinterpret_cast<damage_fn>(get_vfunc(m_vtbl, 0x48))(this, nullptr, move, direction);
        if (effect == 2) {
            event_manager::raise_event(event::FED_UPON, field_C->my_handle);
            if (source) {
                source->damage_ifc();
                auto *damage = field_30->field_C->damage_ifc();
                if (damage && damage->is_subdued()) {
                    auto *combat = static_cast<combat_inode *>(source->get_ai_core()->get_info_node(default_id, true));
                    using clear_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<actor>);
                    reinterpret_cast<clear_fn>(get_vfunc(combat->m_vtbl, 0x108))(
                        combat, nullptr, vhandle_type<actor>{field_C->my_handle});
                }
            }
        }
    } else if (effect == 1) {
        field_30->apply_subdue(attacker, bit_cast<float>(move.field_14.field_20), direction);
    }
    using pending_fn = void(__fastcall *)(combat_inode *, void *, incoming_move);
    reinterpret_cast<pending_fn>(get_vfunc(m_vtbl, 0xD8))(this, nullptr, move);
}

bool combat_inode::find_attack_wall(entity_base *target)
{
    field_D0 = false;
    line_info from_actor;
    line_info from_target;
    const vector3d origin = field_C->get_abs_position();

    vector3d target_position{static_cast<from_mash_in_place_constructor *>(nullptr)};
    if (target)
        target_position = target->get_abs_position();
    const vector3d direction = field_24->get_axis(static_cast<controller_inode::eControllerAxis>(2));
    vector3d side{direction.z * UP.y - direction.y * UP.z,
                  direction.x * UP.z - direction.z * UP.x,
                  direction.y * UP.x - direction.x * UP.y};
    if (side.length2() > 9.999999439624929e-11f)
        side = side / std::sqrt(side.length2());
    using radius_fn = float(__fastcall *)(actor *);
    const float radius = reinterpret_cast<radius_fn>(get_vfunc(field_C->m_vtbl, 0x254))(field_C);
    const auto collide = [](line_info &line) {
        return line.check_collision(
            *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
    };
    const auto accept = [&] {
        *reinterpret_cast<vector3d *>(&field_B8) = from_actor.hit_pos;
        *reinterpret_cast<vector3d *>(&field_C4) = from_actor.hit_norm;
        field_D0 = true;
        return true;
    };
    for (int i = 0; i < 5; ++i) {
        from_actor.field_0 = origin;
        from_actor.field_C = origin + direction * 12.0f;
        if (i == 1) {
            from_actor.field_0 += UP * (radius * 0.5f);
            from_actor.field_C += UP * radius;
        } else if (i == 2) {
            from_actor.field_0 -= UP * (radius * 0.125f);
            from_actor.field_C -= UP * (radius * 0.25f);
        } else if (i == 3) {
            from_actor.field_C += side * radius;
        } else if (i == 4) {
            from_actor.field_C -= side * radius;
        }
        if (!collide(from_actor))
            continue;
        from_target.field_0 = target_position;
        from_target.field_C = from_actor.hit_pos - from_actor.hit_norm * 0.01f;
        if ((from_target.field_C - from_target.field_0).length2() > 144.0f || !collide(from_target) ||
            from_actor.hit_entity.field_0 != from_target.hit_entity.field_0)
            continue;
        const auto &normal = from_actor.hit_norm;
        const auto &other_normal = from_target.hit_norm;
        const float alignment = normal.x * other_normal.x + normal.y * other_normal.y + normal.z * other_normal.z;
        if (alignment > 0.75f && (from_actor.hit_pos - from_target.hit_pos).length2() < 1.0f)
            return accept();

        const vector3d axis{normal.y * other_normal.z - normal.z * other_normal.y,
                            normal.z * other_normal.x - normal.x * other_normal.z,
                            normal.x * other_normal.y - normal.y * other_normal.x};
        if (axis.length2() < 0.01f)
            continue;
        const double first_d =
            -(normal.x * from_actor.hit_pos.x + normal.y * from_actor.hit_pos.y + normal.z * from_actor.hit_pos.z);
        const double second_d = -(other_normal.x * from_target.hit_pos.x + other_normal.y * from_target.hit_pos.y +
                                  other_normal.z * from_target.hit_pos.z);
        const double denominator = static_cast<double>(alignment) * alignment - 1.0;
        const float first_weight = static_cast<float>((alignment * second_d - first_d) / denominator);
        const double second_weight = (alignment * first_d - second_d) / denominator;
        const vector3d plane_origin{static_cast<float>(normal.x * first_weight + other_normal.x * second_weight),
                                    static_cast<float>(normal.y * first_weight + other_normal.y * second_weight),
                                    static_cast<float>(normal.z * first_weight + other_normal.z * second_weight)};
        const auto to_hit = from_actor.hit_pos - plane_origin;
        const float projection = axis.x * to_hit.x + axis.y * to_hit.y + axis.z * to_hit.z;
        const vector3d shared_point = plane_origin + axis * projection;
        from_actor.field_C = from_actor.hit_pos + (shared_point - from_actor.hit_pos) * 0.99f - normal * 0.01f;
        from_target.field_C = from_target.hit_pos + (shared_point - from_target.hit_pos) * 0.99f - other_normal * 0.01f;
        if ((from_actor.field_C - from_actor.field_0).length() < 12.0f &&
            (from_target.field_C - from_target.field_0).length() < 12.0f && collide(from_actor) &&
            collide(from_target) && from_actor.hit_entity.field_0 == from_target.hit_entity.field_0)
            return accept();
    }
    return false;
}

combat_inode::incoming_move::incoming_move()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[344]);
    this->field_4 = 0;
    this->field_10 = 0;
    this->field_90 = false;
}

combat_inode::incoming_move::incoming_move(from_mash_in_place_constructor *tag) : field_14(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[344]);
    field_4 = field_10 = 0;
}

namespace {
using incoming_move = combat_inode::incoming_move;

void __fastcall incoming_destruct(incoming_move *self, void *)
{
    self->field_14.field_4.destruct_mashed_class();
    self->field_14.field_8.destruct_mashed_class();
    self->field_14.field_C.destruct_mashed_class();
    self->field_14.field_10.destruct_mashed_class();
}
void __fastcall incoming_unmash(incoming_move *self, void *, mash_info_struct *info, void *owner)
{
    self->_unmash(info, owner);
}
void *__fastcall incoming_delete(incoming_move *self, void *, unsigned flags)
{
    self->~incoming_move();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall incoming_type(incoming_move *, void *)
{
    return 344;
}
bool __fastcall incoming_subclass(incoming_move *, void *, unsigned type)
{
    return type == 573;
}
bool __fastcall incoming_is_or_subclass(incoming_move *, void *, unsigned type)
{
    return type == 344 || type == 573;
}
int __fastcall incoming_size(incoming_move *, void *)
{
    return sizeof(incoming_move);
}
}

void *combat_inode::incoming_move::native_vtable()
{
    static void *table[] = {
        reinterpret_cast<void *>(&incoming_destruct),
        reinterpret_cast<void *>(&incoming_unmash),
        reinterpret_cast<void *>(&incoming_delete),
        reinterpret_cast<void *>(&incoming_type),
        reinterpret_cast<void *>(&incoming_subclass),
        reinterpret_cast<void *>(&incoming_is_or_subclass),
        reinterpret_cast<void *>(&incoming_size),
    };
    return table;
}

combat_inode::incoming_move::incoming_move(const ai::combat_inode::incoming_move &a2) : field_14(a2.field_14)
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[344]);
    this->field_4 = a2.field_4;
    this->field_8 = a2.field_8;
    this->field_C = a2.field_C;
    this->field_10 = a2.field_10;
    this->field_90 = a2.field_90;
}

combat_inode::combat_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[342]);
    field_1C = field_20 = 0;
    field_50 = 0;
    field_70 = 0;
    field_78 = 0;
    field_80 = field_81 = field_82 = field_83 = field_84 = field_85 = field_86 = false;
    field_90 = field_94 = field_98 = 0;
    field_A0 = 0;
    field_B0 = 0;
    field_B4 = field_D0 = false;
    field_D4 = 0;
    field_7C = field_74 = 0;
}

combat_inode::combat_inode(from_mash_in_place_constructor *tag)
    : info_node(tag), field_D8{incoming_move(tag), incoming_move(tag), incoming_move(tag), incoming_move(tag)}
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[342]);
    field_1C = field_20 = 0;
    field_78 = 0;
    field_A0 = 0;
    field_7C = field_74 = 0;
}

void combat_inode::incoming_move::_unmash(mash_info_struct *info, void *)
{
    mash_virtual_base::fixup_vtable(&field_14);
    field_14.unmash(info, this);
}

void combat_inode::_unmash(mash_info_struct *info, void *owner)
{
    info_node::_unmash(info, owner);
    reinterpret_cast<string_hash &>(field_70).unmash(info, this);
    reinterpret_cast<string_hash &>(field_90).unmash(info, this);
    reinterpret_cast<string_hash &>(field_94).unmash(info, this);
    reinterpret_cast<string_hash &>(field_98).unmash(info, &field_98);
    reinterpret_cast<string_hash &>(field_D4).unmash(info, this);
    for (auto &move : field_D8) {
        mash_virtual_base::fixup_vtable(&move);
        move._unmash(info, this);
    }
}

combo_system_move *combat_inode::get_cur_move()
{
    return field_8->field_6C->field_10->field_0.m_data[static_cast<uint16_t>(field_40)];
}

combo_system_move *combat_inode::get_next_move()
{
    return field_8->field_6C->field_10->field_0.m_data[static_cast<uint16_t>(field_44)];
}

void combat_inode::_activate(ai_core *a2)
{
    auto v2 = 100.0f;
    this->field_54 = 0.0;
    this->field_34 = nullptr;
    auto v4 = v2 < this->field_5C;
    this->field_60 = 100.0f;
    if (v4) {
        this->field_60 = this->field_5C;
        this->field_5C = 100.0f;
    }

    if (this->field_58 > this->field_60) {
        this->field_58 = this->field_60;
    }

    if (this->field_58 < this->field_5C) {
        this->field_58 = this->field_5C;
    }

    auto v5 = this->field_60;
    this->field_5C = 0.0f;
    if (v5 < 0.0f) {
        auto v6 = this->field_60;
        this->field_60 = 0.0f;
        this->field_5C = v6;
    }

    if (this->field_58 > (double)this->field_60) {
        this->field_58 = this->field_60;
    }

    if (this->field_58 < (double)this->field_5C) {
        this->field_58 = this->field_5C;
    }

    float v7 = 0.0f;
    this->field_58 = 0.0f;
    if (v7 > this->field_60) {
        this->field_58 = this->field_60;
    }

    if (this->field_58 < this->field_5C) {
        this->field_58 = this->field_5C;
    }

    auto *v8 = a2;
    info_node::_activate(a2);
    this->clear_cur_move();
    this->clear_next_move();
    this->field_6C = 0.0;
    this->field_78 = 0;
    this->field_7C = 0;
    this->field_B4 = 0;
    this->field_82 = 0;
    this->field_83 = 0;
    this->field_88 = -1;
    this->field_8C = -1;
    this->field_85 = 0;
    this->field_86 = 0;

    int v15 = 0;
    auto v9 = v8->field_50.get_optional_pb_int(combat_inode::reject_all_hash, v15, nullptr);
    this->field_81 = v9 != 0;

    v15 = 0;
    auto v10 = v8->field_50.get_optional_pb_int(combat_inode::always_keep_target_hash, v15, nullptr);
    this->field_80 = v10 != 0;

    this->field_24 = (controller_inode *)v8->get_info_node(controller_inode::default_id, false);

    this->field_28 = (als_inode *)v8->get_info_node(als_inode::default_id, true);

    this->field_2C = (combat_target_inode *)v8->get_info_node(combat_target_inode::default_id, false);

    this->field_30 = (damage_inode *)v8->get_info_node(damage_inode::default_id, true);
}

bool combat_inode::needs_hit_react(Float)
{
    using index_fn = int(__fastcall *)(combat_inode *);
    return ((!field_82 || field_85) && field_88 != -1 && !is_ignored_hit_react_category(string_hash{field_90})) ||
           reinterpret_cast<index_fn>(get_vfunc(m_vtbl, 0x38))(this) != -1;
}

bool combat_inode::is_ignored_hit_react_category(string_hash category) const
{
    for (int i = 1;; ++i) {
        char name[48];
        std::snprintf(name, sizeof(name), "ignore_hit_react_category%d", i);
        const string_hash parameter{int(to_hash(name))};
        if (!my_param_block.does_parameter_exist(parameter))
            return false;
        if (my_param_block.get_pb_hash(parameter) == category)
            return true;
    }
}

int combat_inode::get_react_index()
{
    for (int i = 0; i != 4; ++i) {
        const auto &move = field_D8[i];
        if (move.field_10 != 1)
            continue;
        const float delta = g_world_ptr->time_manager.field_10;
        double lower = 0.000001f;
        if (field_82) {
            lower = 0.033333335f - delta * 0.5;
            if (lower < 0.0)
                lower = 0.000001f;
        }
        const float upper = std::max(0.033333335f, delta * 0.55f) + 0.033333335f - 0.000001f;
        if (lower <= move.field_C && move.field_C <= upper) {
            const vhandle_type<actor> source{entity_base_vhandle{static_cast<uint32_t>(move.field_4)}};
            actor *attacker = source.get_volatile_ptr();
            if (move.field_90) {
                const bool category_allowed = !is_ignored_hit_react_category(move.field_14.field_8);
                if (attacker && static_cast<unsigned>(move.field_14.field_28) <= 2 && category_allowed)
                    return i;
            } else if (attacker) {
                auto *combat = static_cast<combat_inode *>(attacker->get_ai_core()->get_info_node(default_id, true));
                using clear_fn = void(__fastcall *)(combat_inode *, void *, vhandle_type<actor>);
                reinterpret_cast<clear_fn>(get_vfunc(combat->m_vtbl, 0x108))(
                    combat, nullptr, vhandle_type<actor>{field_8->field_64->my_handle});
            }
        }
    }
    return -1;
}

int combat_inode::get_avoid_index()
{
    for (int i = 0; i != 4; ++i) {
        const auto &move = field_D8[i];
        if (move.field_10 != 1)
            continue;
        const float delta = g_world_ptr->time_manager.field_10;
        const float window = std::max(0.033333335f, delta * 0.55f);
        if (window + 0.03333433344960213f <= move.field_C &&
            move.field_C <= delta * 1.9900000095367432f + 0.13333334028720856f)
            return i;
    }
    return -1;
}

void combat_inode::consider_incoming_move_forced_responses()
{
    using index_fn = int(__fastcall *)(combat_inode *);
    using response_fn = void(__fastcall *)(combat_inode *,
                                           void *,
                                           string_hash,
                                           string_hash,
                                           string_hash,
                                           int,
                                           vhandle_type<actor>,
                                           const vector3d &,
                                           bool,
                                           bool);
    const auto consider = [this](int index) {
        const auto &move = field_D8[index];
        const vector3d normal{0.0f, 0.0f, 0.0f};
        reinterpret_cast<response_fn>(get_vfunc(m_vtbl, 0x128))(
            this,
            nullptr,
            move.field_14.field_8,
            move.field_14.field_4,
            move.field_14.field_C,
            move.field_14.field_24,
            vhandle_type<actor>{entity_base_vhandle{static_cast<uint32_t>(move.field_4)}},
            normal,
            true,
            false);
    };
    const int avoid = reinterpret_cast<index_fn>(get_vfunc(m_vtbl, 0x34))(this);
    if (avoid >= 0)
        consider(avoid);
    const int react = reinterpret_cast<index_fn>(get_vfunc(m_vtbl, 0x38))(this);
    if (react >= 0 && react != avoid)
        consider(react);
}

void combat_inode::clear_forced_react_needed()
{
    field_88 = -1;
    field_85 = false;
    if (field_8C == -1) {
        field_98 = 0;
        field_9C = 18;
        field_A0 = 0;
        field_A4 = field_A8 = 0;
        field_AC = 0x3F800000;
    }
}

void combat_inode::clear_forced_avoid_needed()
{
    field_8C = -1;
    field_86 = false;
    if (field_88 == -1) {
        field_98 = 0;
        field_9C = 18;
        field_A0 = 0;
        field_A4 = field_A8 = 0;
        field_AC = 0x3F800000;
    }
}

void combat_inode::clear_from_target(vhandle_type<actor> target)
{
    if (static_cast<uint32_t>(field_1C) == target.field_0.field_0)
        field_1C = 0;
    if (static_cast<uint32_t>(field_20) == target.field_0.field_0)
        field_20 = 0;
    using hero_fn = bool(__fastcall *)(actor *);
    if (reinterpret_cast<hero_fn>(get_vfunc(field_C->m_vtbl, 0x4C))(field_C))
        field_54 = 0.0f;
}

void combat_inode::_frame_advance(Float delta)
{
    using predicate_fn = bool(__fastcall *)(combat_inode *);
    using action_fn = void(__fastcall *)(combat_inode *);
    using move_fn = combo_system_move *(__fastcall *)(combat_inode *);
    field_84 = false;
    if (!reinterpret_cast<predicate_fn>(get_vfunc(m_vtbl, 0xA4))(this) &&
        !reinterpret_cast<predicate_fn>(get_vfunc(m_vtbl, 0xB4))(this) && field_6C > 0.0f) {
        field_6C -= delta;
        if (field_6C <= 0.0f)
            field_6C = 0.0f;
    }
    field_78 = 0;
    if (reinterpret_cast<predicate_fn>(get_vfunc(m_vtbl, 0x84))(this))
        reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0x9C))(this);
    const int ticks = g_world_ptr->time_manager.field_C;
    const auto expired = [ticks](int stamp) {
        if (stamp == -1)
            return false;
        return ticks > stamp
                   ? ticks - stamp > 3
                   : ticks < stamp && static_cast<uint32_t>(ticks) - static_cast<uint32_t>(stamp) + 0x7FFFFFFFu > 3;
    };
    if ((!field_82 || field_85) && expired(field_88))
        clear_forced_react_needed();
    if ((!field_83 || field_86) && expired(field_8C))
        clear_forced_avoid_needed();
    if (reinterpret_cast<predicate_fn>(get_vfunc(m_vtbl, 0xB4))(this) &&
        reinterpret_cast<move_fn>(get_vfunc(m_vtbl, 0xB8))(this)->field_4.field_24 == 15) {
        reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0xB0))(this);
        reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0xAC))(this);
    }
    bool removed_stale = false;
    for (auto &move : field_D8) {
        if (static_cast<uint32_t>(ticks) - static_cast<uint32_t>(move.field_8) > 60 && move.field_10) {
            move.field_10 = 0;
            removed_stale = true;
            break;
        }
    }
    if (!removed_stale) {
        for (auto &move : field_D8)
            if (move.field_10 && static_cast<uint32_t>(move.field_8) < static_cast<uint32_t>(ticks - 2))
                move.field_10 = 0;
    }
    reinterpret_cast<action_fn>(get_vfunc(m_vtbl, 0x4C))(this);
    advance_tether(delta);
}

void combat_inode::advance_tether(Float delta)
{
    if (!field_C->has_physical_ifc())
        return;
    auto *constraint = field_C->physical_ifc()->get_pendulum(4);
    if (constraint != nullptr) {
        if (field_34 == nullptr) {
            void *storage = sizeof(combat_tether) <= slab_allocator::get_max_object_size()
                                ? slab_allocator::allocate(sizeof(combat_tether), nullptr)
                                : ::operator new(sizeof(combat_tether));
            field_34 = ::new (storage) combat_tether(constraint->get_pivot_abs_pos());
        }
        vector3d attachment = ZEROVEC;
        if (auto *distance = constraint->biped_physics_constraint) {
            const auto &point = distance->field_C.field_0;
            attachment = po{distance->b1->field_0}.slow_xform(vector3d{point[0], point[1], point[2]});
        }
        auto *tether = static_cast<combat_tether *>(field_34);
        vector3d pivot = constraint->get_pivot_abs_pos();
        if ((pivot - tether->previous_pivot).length2() > 16.0f)
            pivot = tether->previous_pivot;
        tether->field_60 = pivot;
        if (tether->tentacle != nullptr && (tether->field_A8 & 0x100) == 0)
            tether->tentacle->set_abs_position(pivot);
        tether->end_pos = attachment;
        tether->create_line(attachment, nullptr);
        tether->frame_advance(delta);
        tether->update_spline();
        tether->previous_pivot = pivot;
    } else if (field_34 != nullptr) {
        auto *tether = static_cast<combat_tether *>(field_34);
        tether->kill_all_engines();
        tether->~combat_tether();
        if (sizeof(combat_tether) <= slab_allocator::get_max_object_size())
            slab_allocator::deallocate(tether, nullptr);
        else
            ::operator delete(tether);
        field_34 = nullptr;
    }
}

void combat_inode::_clear_cur_move()
{
    this->field_40 = -1;
    this->field_1C = 0;
    this->field_B4 = 0;
    using hero_fn = bool(__fastcall *)(actor *);
    using next_fn = bool(__fastcall *)(combat_inode *);
    if (reinterpret_cast<hero_fn>(get_vfunc(field_C->m_vtbl, 0x4C))(field_C) &&
        !reinterpret_cast<next_fn>(get_vfunc(m_vtbl, 0xB4))(this)) {
        this->field_54 = 0;
    }
}

void combat_inode::clear_cur_move()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xAC));
    func(this);
}

bool combat_inode::has_next_move()
{
    return this->field_44 != -1;
}

void combat_inode::_clear_next_move()
{
    this->field_7C = 3;
    this->field_44 = -1;
    this->field_20 = 0;
}

void combat_inode::clear_next_move()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xBC));
    func(this);
}

void combat_inode::left_air()
{
    this->field_68 = 0;
}

bool combat_inode::set_attack(string_hash attack)
{
    field_70 = attack.source_hash_code;
    auto *system = field_8->field_6C->field_10;
    int index = -1;
    for (int i = 0; i < system->field_14.m_size; ++i)
        if (system->field_14.m_data[i]->field_14 == attack) {
            index = i;
            break;
        }
    if (index >= 0) {
        field_74 = reinterpret_cast<int>(system->field_14.m_data[index]);
        using target_fn = vhandle_type<actor> *(__fastcall *)(combat_target_inode *, void *, vhandle_type<actor> *);
        vhandle_type<actor> target;
        reinterpret_cast<target_fn>(get_vfunc(field_2C->m_vtbl, 0x38))(field_2C, nullptr, &target);
        if (auto *actor = target.get_volatile_ptr()) {
            field_C->get_abs_position();
            actor->get_abs_position();
            Float time{0.0f};
            string_hash category;
            field_28->get_known_combat_signal_time_and_category(time, category);
            field_28->get_category_id(static_cast<als::layer_types>(0));
            field_7C = 1;
            return true;
        }
    }
    field_74 = field_70 = field_7C = 0;
    return false;
}

bool combat_inode::choose_attack(actor *target)
{
    if (target == nullptr)
        return false;
    auto *system = field_8->field_6C->field_10;
    const vector3d displacement = target->get_abs_position() - field_8->field_64->get_abs_position();
    const float horizontal = std::sqrt(displacement.x * displacement.x + displacement.z * displacement.z);
    const float distance = displacement.length();
    std::array<float, 32> weights{};
    int count = 0;
    if (field_28 == nullptr || field_28->is_layer_interruptable(static_cast<als::layer_types>(0))) {
        string_hash category{0};
        if (field_28)
            category = field_28->get_category_id(static_cast<als::layer_types>(0));
        for (int i = 0; i < system->field_14.m_size; ++i) {
            auto *chain = system->field_14.m_data[i];
            assert(i < static_cast<int>(weights.size()));
            ++count;
            if (chain->field_1C.m_size <= 0)
                continue;
            auto *move = system->field_0.m_data[static_cast<uint16_t>(chain->field_1C.m_data[0])];
            const auto &range = move->field_80.field_1C;
            const float *bounds = reinterpret_cast<const float *>(&range.field_4);
            if (distance < bit_cast<float>(chain->field_34) || distance > chain->field_38 || horizontal < bounds[0] ||
                horizontal > bounds[1] || displacement.y < bounds[2] || displacement.y > bounds[3])
                continue;
            bool linked = field_28 == nullptr || move->field_80.field_30.m_size == 0;
            if (!linked)
                for (int j = 0; j < move->field_80.field_30.m_size; ++j)
                    if (move->field_80.field_30.m_data[j]->field_4 == category) {
                        linked = true;
                        break;
                    }
            if (linked)
                weights[i] = chain->field_3C;
        }
    }
    double total = 0.0;
    for (int i = 0; i < count; ++i)
        if (weights[i] > 0.0f)
            total += weights[i];
    int selected = -1;
    if (total > 0.0) {
        const float choice = static_cast<float>(std::rand()) / 32768.0f * static_cast<float>(total);
        double cumulative = 0.0;
        for (int i = 0; i < count; ++i)
            if (weights[i] > 0.0f) {
                cumulative += weights[i];
                if (choice <= cumulative) {
                    selected = i;
                    break;
                }
            }
    }
    if (selected < 0) {
        field_74 = field_70 = field_7C = 0;
        return false;
    }
    auto *chain = system->field_14.m_data[static_cast<uint16_t>(selected)];
    field_74 = reinterpret_cast<int>(chain);
    field_70 = chain->field_14.source_hash_code;
    field_7C = 1;
    return true;
}

float combat_inode::get_attack_min_distance() const
{
    return field_74 ? bit_cast<float>(reinterpret_cast<const combo_system_chain *>(field_74)->field_2C) : 0.0f;
}

float combat_inode::get_attack_max_distance() const
{
    return field_74 ? bit_cast<float>(reinterpret_cast<const combo_system_chain *>(field_74)->field_30) : 1.0f;
}

}  // namespace ai
