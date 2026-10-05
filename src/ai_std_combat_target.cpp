#include "ai_std_combat_target.h"

#include "common.h"
#include "func_wrapper.h"
#include "vtbl.h"
#include "ai_team.h"
#include "base_ai_core.h"
#include "damage_interface.h"
#include "line_info.h"
#include "local_collision.h"
#include "oldmath_po.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <array>
#include "ai_player_controller.h"
#include "ai_pedestrian.h"
#include "combat_inode.h"
#include "core_ai_resource.h"
#include "game_button.h"
#include "ped_spawner.h"

VALIDATE_SIZE(ai::combat_target_inode, 0x88);
VALIDATE_SIZE(ai::venom_combat_target_inode, 0x94);

namespace {
unsigned __fastcall combat_target_type(ai::combat_target_inode *, void *) { return 351; }
bool __fastcall combat_target_subclass(ai::combat_target_inode *, void *, unsigned type)
{
    return type == 349 || type == 350 || type == 537 || type == 573;
}
void __fastcall combat_target_activate(ai::combat_target_inode *self, void *, ai::ai_core *core)
{
    self->_activate(core);
}
int __fastcall combat_target_size(ai::combat_target_inode *, void *) { return sizeof(ai::combat_target_inode); }
bool __fastcall combat_target_plausible(ai::combat_target_inode *self, void *, vhandle_type<actor> candidate)
{
    return self->target_plausible(candidate);
}
}

void *ai::combat_target_inode::native_vtable()
{

    static auto table = [] {
        std::array<void *, 39> result;
        std::copy_n(static_cast<void **>(base_full_target_inode::native_vtable()), result.size(), result.data());
        result[3] = reinterpret_cast<void *>(&combat_target_type);
        result[4] = reinterpret_cast<void *>(&combat_target_subclass);
        result[8] = reinterpret_cast<void *>(&combat_target_activate);
        result[11] = reinterpret_cast<void *>(&combat_target_size);
        result[0x44 / 4] = reinterpret_cast<void *>(&combat_target_plausible);
        return result;
    }();
    return table.data();
}

namespace {
unsigned __fastcall venom_target_type(ai::venom_combat_target_inode *, void *) { return 356; }
bool __fastcall venom_target_subclass(ai::venom_combat_target_inode *, void *, unsigned type)
{
    return type == 353 || type == 351 || type == 349 || type == 350 || type == 537 || type == 573;
}
void __fastcall venom_target_advance(ai::venom_combat_target_inode *self, void *, Float delta)
{
    self->_frame_advance(delta);
}
int __fastcall venom_target_size(ai::venom_combat_target_inode *, void *) { return sizeof(ai::venom_combat_target_inode); }
vector3d *__fastcall venom_target_direction(ai::venom_combat_target_inode *self, void *, vector3d *out)
{
    *out = self->get_look_direction();
    return out;
}
vhandle_type<actor> *__fastcall venom_target_get(ai::venom_combat_target_inode *self, void *, vhandle_type<actor> *out)
{
    *out = self->get_player_target();
    return out;
}
vhandle_type<actor> *__fastcall venom_target_find(ai::venom_combat_target_inode *self, void *, vhandle_type<actor> *out)
{
    *out = self->player_style_get_target();
    return out;
}
bool is_targetable_ped(ped_spawner *spawner)
{
    auto *ped = spawner->get_my_actor();
    if (!ped)
        return false;
    auto *core = ped->get_ai_core();
    auto *inode = static_cast<ai::pedestrian_inode *>(core->get_info_node(ai::pedestrian_inode::default_id, true));
    if (inode->field_D1)
        return false;
    const string_hash team_key{int(to_hash("team"))};
    bool targetable_team = false;
    if (core->field_50.does_parameter_exist(team_key)) {
        const auto team = ai::team::manager::get_team_enum_by_hash(core->field_50.get_pb_hash(team_key));
        targetable_team = static_cast<int>(team) == 13 || static_cast<int>(team) == 14;
    }
    return !spawner->field_5 && targetable_team;
}
}

void *ai::venom_combat_target_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 39> result;
        std::copy_n(static_cast<void **>(combat_target_inode::native_vtable()), result.size(), result.data());
        result[3] = reinterpret_cast<void *>(&venom_target_type);
        result[4] = reinterpret_cast<void *>(&venom_target_subclass);
        result[7] = reinterpret_cast<void *>(&venom_target_advance);
        result[11] = reinterpret_cast<void *>(&venom_target_size);
        result[0x38 / 4] = reinterpret_cast<void *>(&venom_target_get);
        result[0x48 / 4] = reinterpret_cast<void *>(&venom_target_direction);
        result[0x54 / 4] = reinterpret_cast<void *>(&venom_target_find);
        return result;
    }();
    return table.data();
}

vhandle_type<actor> ai::venom_combat_target_inode::get_player_target()
{
    auto *controller = field_C->get_player_controller();
    const int mode = controller->get_spidey_loco_mode();
    vector3d direction = get_controller_look_direction();
    if (direction.length2() < EPSILON) {
        direction = field_C->get_abs_po().get_z_facing();
        if (mode != 1 && mode != 2)
            direction *= 0.5f;
    } else {
        field_55 = true;
    }
    if (mode != 1 && mode != 2) {
        direction -= field_C->get_abs_po().get_y_facing();
        direction.normalize();
    }
    field_88 = direction.x;
    field_8C = direction.y;
    field_90 = direction.z;
    const auto *button = controller->get_gb_range();
    if (!button->is_flagged(0x20) && button->is_flagged(1))
        field_84 = true;
    using find_fn = vhandle_type<actor> *(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor> *);
    vhandle_type<actor> result;
    reinterpret_cast<find_fn>(get_vfunc(m_vtbl, 0x54))(this, nullptr, &result);
    return result;
}

vhandle_type<actor> ai::venom_combat_target_inode::player_style_get_target()
{
    auto result = find_target();
    if (!result.get_volatile_ptr() || field_38 > field_20) {
        const auto *button = field_C->get_player_controller()->get_gb_range();
        if (!button->is_flagged(0x20) && button->is_flagged(1)) {
            using range_fn = float(__fastcall *)(base_full_target_inode *);
            const float range = reinterpret_cast<range_fn>(get_vfunc(m_vtbl, 0x84))(this);
            float best_distance = range * range;
            for (auto *spawner : ped_spawner::ped_spawner_list) {
                auto *ped = spawner->get_my_actor();
                if (!ped || !is_targetable_ped(spawner))
                    continue;
                const float distance = (ped->get_abs_po().get_position() - field_C->get_abs_po().get_position()).length2();
                if (distance < best_distance) {
                    auto *core = ped->get_ai_core();
                    auto *combat = core ? static_cast<combat_inode *>(core->get_info_node(combat_inode::default_id, false)) : nullptr;
                    if (!combat || !combat->field_82) {
                        using calculate_fn = void(__fastcall *)(base_full_target_inode *, void *, vhandle_type<actor>);
                        reinterpret_cast<calculate_fn>(get_vfunc(m_vtbl, 0x50))(this, nullptr, vhandle_type<actor>{ped->my_handle});
                        if (field_34.get_volatile_ptr())
                            best_distance = distance;
                    }
                }
            }
            result = field_34;
            if (field_20 >= field_38) {
                if (auto *target = result.get_volatile_ptr()) {
                    if (auto *core = target->get_ai_core()) {
                        if (core->field_6C->field_44)
                            core->field_4C |= 1;
                    }
                }
            }
        }
    }
    return field_20 < field_38 ? vhandle_type<actor>{0} : result;
}

ai::combat_target_inode::combat_target_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[351]);
    field_84 = false;
}

ai::combat_target_inode::combat_target_inode(from_mash_in_place_constructor *tag)
    : base_full_target_inode(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[351]);
}

ai::venom_combat_target_inode::venom_combat_target_inode() : combat_target_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[356]);
}

ai::venom_combat_target_inode::venom_combat_target_inode(from_mash_in_place_constructor *tag)
    : combat_target_inode(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[356]);
}

mVectorBasic<vhandle_type<actor>> &ai::combat_target_inode::combat_list()
{
    if constexpr (STANDALONE_SYSTEM) {
        static mVectorBasic<vhandle_type<actor>> list;
        return list;
    } else {
        return var<mVectorBasic<vhandle_type<actor>>>(0x0096C2E4);
    }
}

void ai::combat_target_inode::_activate(ai_core *core)
{
    field_84 = false;
    field_1C = &combat_list();
    base_full_target_inode::_activate(core);
    if (!my_param_block.get_optional_pb_int(string_hash{int(to_hash("register_as_targetable"))}, 1, nullptr)) {
        using unregister_fn = void(__fastcall *)(base_full_target_inode *);
        reinterpret_cast<unregister_fn>(get_vfunc(m_vtbl, 0x30))(this);
    }
}

void ai::venom_combat_target_inode::_frame_advance(Float delta)
{
    field_84 = false;
    const vector3d forward = field_C->get_abs_po().get_z_facing();
    field_88 = forward.x;
    field_8C = forward.y;
    field_90 = forward.z;
    using advance_fn = void(__fastcall *)(base_full_target_inode *, void *, Float);
    reinterpret_cast<advance_fn>(get_vfunc(m_vtbl, 0x58))(this, nullptr, delta);
    const vector3d look = get_controller_look_direction();
    field_88 = look.x;
    field_8C = look.y;
    field_90 = look.z;
}


void ai::player_web_target_inode::add_to_web_targets_list(vhandle_type<actor> a1)
{
    web_targets_list.push_back(a1);
}

bool ai::combat_target_inode::target_plausible(vhandle_type<actor> candidate)
{
    if (candidate.field_0 == field_C->my_handle)
        return false;
    auto *target = candidate.get_volatile_ptr();
    if (!target)
        return false;
    using damage_query = bool(__fastcall *)(actor *);
    if (reinterpret_cast<damage_query>(get_vfunc(target->m_vtbl, 0x114))(target)) {
        const auto *damage = target->damage_ifc();
        if ((damage->field_1FC.field_0[0] <= 0.0f && !field_84) || damage->is_subdued())
            return false;
    }
    auto *target_core = target->get_ai_core();
    if (!target_core)
        return false;
    const string_hash team_key{int(to_hash("team"))};
    const auto observer_team = team::manager::get_team_enum_by_hash(field_8->field_50.get_pb_hash(team_key));
    const auto candidate_team = team::manager::get_team_enum_by_hash(target_core->field_50.get_pb_hash(team_key));
    return team::manager::is_enemy(observer_team, candidate_team);
}

void ai::base_full_target_inode::calc_and_update_target(vhandle_type<actor> candidate)
{
    auto *target = candidate.get_volatile_ptr();
    if (!target)
        return;
    vector3d origin = field_C->get_abs_position();
    const vector3d position = target->get_abs_position();
    using bool_query = bool(__fastcall *)(base_full_target_inode *);
    using float_query = float(__fastcall *)(base_full_target_inode *);
    using direction_query = vector3d *(__fastcall *)(base_full_target_inode *, void *, vector3d *);
    if (reinterpret_cast<bool_query>(get_vfunc(m_vtbl, 0x7C))(this)) {
        const auto delta = position - origin;
        const double distance_squared = static_cast<double>(delta.x) * delta.x +
            static_cast<double>(delta.y) * delta.y + static_cast<double>(delta.z) * delta.z;
        if (distance_squared < static_cast<double>(field_48) * field_48) {
            field_34 = candidate;
            field_38 = 3;
            field_48 = static_cast<float>(std::sqrt(distance_squared));
            last_known_position = position;
        }
        return;
    }

    using damage_query = bool(__fastcall *)(actor *);
    const bool dead = reinterpret_cast<damage_query>(get_vfunc(target->m_vtbl, 0x114))(target) &&
        !target->damage_ifc()->is_alive();
    vector3d facing;
    reinterpret_cast<direction_query>(get_vfunc(m_vtbl, 0x48))(this, nullptr, &facing);
    auto direction = position - (origin - facing);
    float distance = direction.length();
    if (distance > 0.01f)
        direction /= distance;
    float facing_dot = dot(direction, facing);
    if (field_5C)
        origin.y += 1.5f;

    int rank = 7;
    float ranked_distance = std::numeric_limits<float>::max();
    if (reinterpret_cast<float_query>(get_vfunc(m_vtbl, 0x84))(this) > distance) {
        if (dead) {
            distance *= 0.75f;
            facing_dot = static_cast<float>(facing_dot + 0.1);
        }
        const auto center = origin + facing * (ellipse_length * 0.5f);
        const auto sideways = vector3d::cross(facing, vector3d{0.0f, 1.0f, 0.0f});
        const auto vertical = vector3d::cross(facing, sideways);
        auto offset = position - center;
        if (dead)
            offset *= 0.9f;
        const float along = dot(facing, offset) / (ellipse_length * 0.5f);
        const float across = dot(sideways, offset) / (ellipse_width * 0.5f);
        const float above = dot(vertical, offset) / (ellipse_width * 0.5f);
        if (along * along + across * across + above * above < 1.0f)
            rank = 0;
        else if (facing_dot > close_vision_angle_cos && distance < close_vision_range)
            rank = 1;
        else if (reinterpret_cast<float_query>(get_vfunc(m_vtbl, 0x80))(this) < facing_dot &&
                 reinterpret_cast<float_query>(get_vfunc(m_vtbl, 0x84))(this) > distance)
            rank = 2;
        else if (distance < touch_range)
            rank = 3;
    }
    if (rank == 7 && reinterpret_cast<float_query>(get_vfunc(m_vtbl, 0x88))(this) > distance)
        rank = 4;


    if (rank < 4 || (rank == 4 && distance < 95.0f)) {
        ranked_distance = distance;
        if (rank < field_38 || (rank == field_38 && distance < field_48)) {
            line_info sight{origin, position};
            if (sight.check_collision(*local_collision::entfilter_blocks_ai_los,
                                      *local_collision::obbfilter_lineseg_test, nullptr))
                rank = 7;
        }
    }
    if (rank == 7 && reinterpret_cast<float_query>(get_vfunc(m_vtbl, 0x8C))(this) > distance) {
        rank = 5;
        ranked_distance = distance;
    }
    if (rank < field_38 || (rank == field_38 && ranked_distance < field_48)) {
        field_38 = rank;
        field_48 = ranked_distance;
        field_34 = candidate;
        last_known_position = position;
    }
}
