#include "ai_state_run.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_std_hero.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "base_full_target_inode.h"
#include "camera.h"
#include "combat_inode.h"
#include "common.h"
#include "controller_inode.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "game.h"
#include "game_settings.h"
#include "info_node_desc_list.h"
#include "input.h"
#include "inputsettings.h"
#include "line_info.h"
#include "mashed_state.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "physics_inode.h"
#include "sound_and_pfx_interface.h"
#include "state_machine.h"
#include "utility.h"
#include "variables.h"

#include <algorithm>
#include <cmath>
#include <array>

namespace ai {

VALIDATE_SIZE(run_state, 0x5C);
VALIDATE_OFFSET(run_state, field_50, 0x50);

namespace {
void *__fastcall run_delete(run_state *self, void *, unsigned char flags)
{
    self->~run_state();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(*self));
    return self;
}
unsigned __fastcall run_type(run_state *, void *) { return run_state::virtual_type; }
bool __fastcall run_subclass(run_state *, void *, unsigned type)
{
    return type == 535 || type == 567 || type == 573;
}
int __fastcall run_size(run_state *, void *) { return sizeof(run_state); }
void __fastcall run_activate(run_state *self, void *, ai_state_machine *machine,
    const mashed_state *state, const mashed_state *previous, const param_block *parameters,
    base_state::activate_flag_e flags)
{
    self->_activate(machine, state, previous, parameters, flags);
}
void __fastcall run_deactivate(run_state *self, void *, const mashed_state *next)
{
    self->_deactivate(next);
}
state_trans_messages __fastcall run_advance(run_state *self, void *, Float elapsed)
{
    return self->_frame_advance(elapsed);
}
void __fastcall run_nodes(run_state *self, void *, info_node_desc_list *list)
{
    self->_get_info_node_list(*list);
}
}

void *run_state::native_vtable()
{
    static const auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), result.size(), result.begin());
        result[2] = reinterpret_cast<void *>(&run_delete);
        result[3] = reinterpret_cast<void *>(&run_type);
        result[4] = reinterpret_cast<void *>(&run_subclass);
        result[6] = reinterpret_cast<void *>(&run_activate);
        result[7] = reinterpret_cast<void *>(&run_deactivate);
        result[8] = reinterpret_cast<void *>(&run_advance);
        result[9] = reinterpret_cast<void *>(&run_nodes);
        result[13] = reinterpret_cast<void *>(&run_size);
        return result;
    }();
    return const_cast<void **>(table.data());
}

run_state::run_state() : enhanced_state(), field_58(false)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

run_state::run_state(from_mash_in_place_constructor *tag) : enhanced_state(tag), field_30(tag), field_3C(tag)
{

    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[virtual_type]);
}

void run_state::_get_info_node_list(info_node_desc_list &list)
{
    list.add_entry({hero_inode::default_id, 384});
    list.add_entry({als_inode::default_id, 333});
    list.add_entry({controller_inode::default_id, 358});
}

void run_state::_activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                          const param_block *params, activate_flag_e flags)
{
    enhanced_state::activate(machine, state, previous, params, flags);
    field_50 = static_cast<hero_inode *>(get_core()->get_info_node(hero_inode::default_id, true));
    auto *animation = field_50->field_20;
    auto *physics = field_50->field_28;
    get_actor()->m_player_controller->set_spidey_loco_mode(static_cast<eHeroLocoMode>(1));
    const auto layer = static_cast<als::layer_types>(0);
    const auto category = animation->get_category_id(layer);
    static const string_hash combat_jump{"Combat_Jump"};
    static const string_hash combat_fall{"Combat_Fall"};
    if (category != combat_jump && category != combat_fall)
        field_50->field_2C->left_air();
    physics->setup_for_walk();
    static const string_hash jump_land{to_hash("Jump_Land")};
    if (animation->get_category_id(layer) != jump_land && animation->is_layer_interruptable(layer))
        animation->request_category_transition(cat_id_idle_walk_run(), layer, true, false, false);

    field_48 = 0.0f;
    field_30 = ZEROVEC;
    field_3C = ZEROVEC;
    field_54 = 0;
    field_4C = 0;
    field_58 = false;
    als::param_list desired;
    desired.add_param(24u, YVEC);
    animation->get_als_layer(layer)->set_desired_params(desired);

    auto *owner = get_actor();
    if (previous && static_cast<unsigned>(previous->field_14) == 303 &&
        owner->has_sound_and_pfx_ifc() && owner->has_physical_ifc()) {
        auto *physical = owner->physical_ifc();
        const float minimum = bit_cast<float>(physical->field_E0);
        const float blend = std::clamp((field_50->field_74 - minimum) / (physical->field_E4 - minimum), 0.0f, 1.0f);
        const float initial = bit_cast<float>(physical->field_DC);
        const float volume = blend * (1.0f - initial) + initial;
        if (volume > 0.0f) {
            string_hash terrain;
            physical->get_parent_terrain_type(&terrain);
            owner->sound_and_pfx_ifc()->play_terrain_sound(static_cast<eTerrainSoundType>(9), terrain, volume);
        }
        static const string_hash takes_fall_damage{"takes_fall_damage"};
        if (get_core()->field_50.get_pb_int(takes_fall_damage) == 1) {
            field_50->rumble_and_damage(field_50->field_58.y - physics->get_abs_position().y);
            field_50->field_58.y = physics->get_abs_position().y;
        }
    }
    if (owner->has_physical_ifc() && owner->physical_ifc()->allow_manage_standing())
        owner->physical_ifc()->manage_standing(false);
}

void run_state::_deactivate(const mashed_state *)
{
    if (g_is_the_packer)
        return;
    auto *owner = get_actor();
    if (owner && owner->has_physical_ifc()) {
        setup_hero_capsule(owner);
        if (field_54 == 1) {
            owner->physical_ifc()->set_allow_manage_standing(true);
            owner->set_terrain_collisions_active(true);
        }
    }
}

bool run_state::check_for_fence_hop(Float, vector3d *)
{
    static const string_hash grind_angle{to_hash("fence_hop_grind_angle_cosine")};
    get_core()->field_50.get_pb_float(grind_angle);
    auto *owner = get_actor();
    auto start = owner->get_abs_position();
    start.y -= 0.1f;
    const auto end = start + owner->get_abs_po().get_z_facing() * 0.8f;
    line_info collision{start, end};
    if (collision.check_collision(*local_collision::entfilter_entity_no_capsules,
                                   *local_collision::obbfilter_lineseg_test, nullptr))
        owner->cancel_animated_movement(collision.hit_norm, 0.0f);
    return false;
}

state_trans_messages run_state::_frame_advance(Float elapsed)
{
    const auto result = enhanced_state::frame_advance(elapsed);
    auto *animation = field_50->field_20;
    auto *controller = field_50->field_24;
    auto *physics = field_50->field_28;
    auto *targeting = field_50->field_30;
    auto *owner = get_actor();
    field_50->field_58.y = physics->get_abs_position().y;
    if (owner->is_frame_delta_valid()) {
        const vector3d delta = owner->get_frame_delta()->m[3];
        const float distance = delta.length();
        if (hero_inode::get_hero_type() == 2)
            g_game_ptr->gamefile->update_miles_run_venom(distance);
        else
            g_game_ptr->gamefile->update_miles_run_spidey(distance);
    }
    owner->m_player_controller->set_spidey_loco_mode(static_cast<eHeroLocoMode>(1));
    const auto layer = static_cast<als::layer_types>(0);
    if (!animation->is_layer_interruptable(layer))
        return TRANS_TOTAL_MSGS;

    const auto axis = static_cast<controller_inode::eControllerAxis>(0);
    const float magnitude = controller->get_axis_2d(axis).length();
    static const string_hash allow_walk_run{to_hash("loco_allow_walk_run")};
    const bool allowed = get_core()->field_50.get_optional_pb_int(allow_walk_run, 0, nullptr) != 0;
    const bool moving = allowed && !controller->is_axis_neutral(axis);
    field_48 = (moving ? magnitude : 0.0f) * 7.0f;
    als::param_list desired;
    desired.add_param({0, field_48});
    static const string_hash lock_on{to_hash("lock_on_target")};
    const int lock_requested = get_core()->field_50.get_pb_int(lock_on);
    const bool locked = targeting->quick_targeting().get_volatile_ptr() && targeting->is_target_known() &&
                        lock_requested && animation->get_category_id(layer) == cat_id_idle_walk_run();
    desired.add_param({52, locked ? 1.0f : 0.0f});

    if (field_54 == 1 && animation->get_als_layer(layer)->get_state_id() == string_hash{}) {
        setup_hero_capsule(owner);
        field_54 = 0;
        owner->physical_ifc()->set_allow_manage_standing(true);
        owner->set_terrain_collisions_active(true);
    }
    if (owner->has_physical_ifc()) {
        const float normal_y = owner->physical_ifc()->field_100.y;
        if (!field_58 && normal_y <= 0.73242188f) {
            shrink_capsule_for_slanted_surfaces(owner);
            field_58 = true;
        } else if (field_58 && normal_y > 0.73242188f) {
            setup_hero_capsule(owner);
            field_58 = false;
        }
    }

    vector3d facing;
    vector3d travel;
    if (!moving) {
        facing = owner->get_abs_po().get_z_facing();
        if (locked) {
            auto position = physics->get_abs_position();
            auto target = targeting->quick_targeting().get_volatile_ptr()->get_abs_position();
            if ((position - field_30).length() < 0.3f && (target - field_3C).length() < 0.3f) {
                position = field_30;
                target = field_3C;
            }
            facing = -(position - target).normalized();
            field_30 = position;
            field_3C = target;
        }
        travel = facing;
    } else {
        travel = controller->get_axis(axis).normalized();
        facing = travel;
        if (locked) {
            const auto target = targeting->quick_targeting().get_volatile_ptr()->get_abs_position();
            facing = -(physics->get_abs_position() - target).normalized();
        }
        check_for_fence_hop(elapsed, &travel);
        const vector3d camera_forward = g_game_ptr->get_current_view_camera(0)->get_abs_po().m[2];
        auto &settings = Input::instance->field_129D8[0]->field_18;
        if (settings.get_state(InputAction::Forward) > 0.8f &&
            settings.get_state(InputAction::Backward) < 0.8f &&
            settings.get_state(InputAction::TurnRight) < 0.8f) {
            facing = camera_forward;
            travel = camera_forward;
        }
    }
    desired.add_param(27u, facing);
    desired.add_param(30u, travel);
    desired.add_param(24u, YVEC);
    animation->get_als_layer(layer)->set_desired_params(desired);
    field_50->cleanup_collision_lists();
    return result;
}

}  // namespace ai

void run_state_patch()
{
    FUNC_ADDRESS(address, &ai::run_state::_frame_advance);
    set_vfunc(0x00877498, address);
}
