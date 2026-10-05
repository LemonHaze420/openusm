#include "plr_loco_crawl_transition_state.h"

#include "actor.h"
#include "ai_player_controller.h"
#include "ai_std_hero.h"
#include "als_animation_logic_system_interface.h"
#include "als_animation_logic_system.h"
#include "als_inode.h"
#include "animation_controller.h"
#include "base_ai_core.h"
#include "colgeom_alter_sys.h"
#include "common.h"
#include "conglom.h"
#include "func_wrapper.h"
#include "info_node_desc_list.h"
#include "info_node_descriptor.h"
#include "nal_anim_controller.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "sound_and_pfx_interface.h"
#include "sound_interface.h"
#include "utility.h"
#include "vtbl.h"
#include "wds.h"

#include <algorithm>
#include <array>
#include <cmath>

VALIDATE_SIZE(plr_loco_crawl_transition_state, 0x34);

namespace {

void set_crawl_transition_pose(actor *owner, vector3d heading)
{
    auto *core = owner->get_ai_core();
    core->get_info_node(ai::als_inode::default_id, true);
    auto *hero = static_cast<ai::hero_inode *>(core->get_info_node(ai::hero_inode::default_id, true));
    const auto &surface = hero->field_1B0;
    const auto transition = static_cast<int>(hero->field_20C.field_0);
    auto position = owner->get_abs_position();
    if (owner->is_a_conglomerate()) {
        static const string_hash pelvis{static_cast<int>(to_hash("BIP01 PELVIS"))};
        position = static_cast<conglomerate *>(owner)->get_member(pelvis, true)->get_abs_position();
    }
    po transform;
    const auto &pose = owner->get_abs_po();
    if (std::abs(dot(pose.get_z_facing(), surface.hit_norm)) > 0.8f && transition != 4) {
        if (transition == 2 || transition == 3 || transition == 5 || transition == 7)
            heading = -heading;
        if (is_colinear(heading, surface.hit_norm, 0.01f)) {
            heading = is_colinear(pose.get_z_facing(), surface.hit_norm, 0.01f)
                ? pose.get_y_facing() : pose.get_z_facing();
        }
        transform.set_po(heading, surface.hit_norm, position);
    } else {
        const auto mode = owner->m_player_controller->get_spidey_loco_mode();
        if ((mode == 2 || mode == 14) && transition >= 1 && transition <= 5) {
            owner->anim_ctrl->get_camera_root_abs_po(transform);
        } else {
            heading = pose.get_z_facing();
            if (is_colinear(heading, surface.hit_norm, 0.01f))
                heading = pose.get_y_facing();
            transform.set_po(heading, surface.hit_norm, position);
        }
    }
    entity_set_abs_po(owner, transform);
    owner->invalidate_frame_delta();
    owner->physical_ifc()->cancel_all_velocity();
    if (owner->has_physical_ifc())
        owner->physical_ifc()->manage_standing(true);
}

uint32_t __fastcall transition_type(const plr_loco_crawl_transition_state *) { return 182; }
bool __fastcall transition_subclass(const plr_loco_crawl_transition_state *, void *, mash::virtual_types_enum type)
{
    return type == 535 || type == 567 || type == 573;
}
int __fastcall transition_size(const plr_loco_crawl_transition_state *) { return 0x34; }
void __fastcall transition_activate(plr_loco_crawl_transition_state *self, void *, ai::ai_state_machine *machine,
    const ai::mashed_state *state, const ai::mashed_state *previous, const ai::param_block *params,
    ai::base_state::activate_flag_e flags)
{
    self->activate(machine, state, previous, params, flags);
}
void __fastcall transition_deactivate(plr_loco_crawl_transition_state *self, void *, const ai::mashed_state *state)
{
    self->deactivate(state);
}
ai::state_trans_messages __fastcall transition_frame(plr_loco_crawl_transition_state *self, void *, Float dt)
{
    return self->frame_advance(dt);
}
void __fastcall transition_list(plr_loco_crawl_transition_state *self, void *, ai::info_node_desc_list &list)
{
    self->get_info_node_list(list);
}
void __fastcall transition_controls(plr_loco_crawl_transition_state *self, void *, int controls)
{
    self->map_controls(controls);
}
void __fastcall transition_mode(plr_loco_crawl_transition_state *self, void *, actor *owner)
{
    self->set_player_mode(owner);
}
}

void *plr_loco_crawl_transition_state::native_vtable()
{

    static auto table = [] {
        std::array<void *, 18> result;
        std::copy_n(static_cast<void **>(ai::enhanced_state::native_vtable()), 16, result.data());
        result[3] = bit_cast<void *>(&transition_type);
        result[4] = bit_cast<void *>(&transition_subclass);
        result[6] = bit_cast<void *>(&transition_activate);
        result[7] = bit_cast<void *>(&transition_deactivate);
        result[8] = bit_cast<void *>(&transition_frame);
        result[9] = bit_cast<void *>(&transition_list);
        result[13] = bit_cast<void *>(&transition_size);
        result[16] = bit_cast<void *>(&transition_controls);
        result[17] = bit_cast<void *>(&transition_mode);
        return result;
    }();
    return table.data();
}

plr_loco_crawl_transition_state::plr_loco_crawl_transition_state()
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}

plr_loco_crawl_transition_state::plr_loco_crawl_transition_state(from_mash_in_place_constructor *tag)
    : ai::enhanced_state(tag)
{
    if constexpr (STANDALONE_SYSTEM)
        m_vtbl = bit_cast<std::intptr_t>(native_vtable());
}

void plr_loco_crawl_transition_state::map_controls(int) { field_30 = 0; }

void plr_loco_crawl_transition_state::set_player_mode(actor *owner)
{
    auto *controller = owner->m_player_controller;
    if (controller->get_spidey_loco_mode() != 14)
        controller->set_spidey_loco_mode(static_cast<eHeroLocoMode>(2));
}

void plr_loco_crawl_transition_state::get_info_node_list(ai::info_node_desc_list &list)
{
    list.add_entry(ai::info_node_descriptor{ai::als_inode::default_id, 333});
}

void plr_loco_crawl_transition_state::activate(ai::ai_state_machine *machine, const ai::mashed_state *state,
    const ai::mashed_state *previous, const ai::param_block *params, ai::base_state::activate_flag_e flags)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0046A340, this, machine, state, previous, params, flags);
        return;
    }
    ai::enhanced_state::activate(machine, state, previous, params, flags);
    auto *owner = get_actor();
    auto *controller = owner->m_player_controller;
    auto *hero = static_cast<ai::hero_inode *>(get_core()->get_info_node(ai::hero_inode::default_id, true));
    vector3d velocity;
    if (owner->has_physical_ifc()) {
        auto *physical = owner->physical_ifc();
        if (hero->field_1B0.hit_norm.y <= 0.732421875f)
            physical->field_C |= 0x200u;
        else
            physical->field_C &= ~0x200u;
        physical->set_allow_manage_standing(true);
        physical->enable(true);
        physical->suspend(false);
    }
    if (auto *capsule = get_core()->field_70) {
        capsule->set_avoid_floor(false);
        capsule->set_mode(static_cast<capsule_alter_sys::eAlterMode>(2));
        capsule->set_static_capsule(vector3d{0.0f, 0.0f, 0.3f}, vector3d{-0.3f, 0.0f, 0.3f}, 0.3f);
    }
    if (owner->has_physical_ifc()) {
        velocity = owner->physical_ifc()->get_velocity();
        owner->physical_ifc()->set_gravity(false);
        owner->physical_ifc()->cancel_all_velocity();
    }
    static const string_hash allow_wallrun{static_cast<int>(to_hash("loco_allow_wallrun"))};
    const bool wallrun = get_core()->field_50.get_pb_int(allow_wallrun) != 0;
    auto heading = YVEC;
    const auto mode = controller->get_spidey_loco_mode();
    if (owner->has_physical_ifc()) {
        heading = orthogonal_projection_onto_plane(velocity, hero->field_1B0.hit_norm);
        if (heading.length2() > LARGE_EPSILON)
            heading.normalize();
        if (mode == 3 || (mode > 4 && mode <= 7)) {
            if (!wallrun || mode == 3 || heading == ZEROVEC || dot(heading, -YVEC) >= 0.8f)
                heading = YVEC;
            if (is_colinear(hero->field_1B0.hit_norm, YVEC, 0.01f))
                heading = owner->get_abs_po().get_z_facing();
            else {
                heading = heading * 0.6f + YVEC * 0.4f;
                heading.normalize();
            }
        } else {
            heading = owner->get_abs_po().get_z_facing();
        }
    }
    if (mode != 2 && mode != 14 && mode != 1)
        set_crawl_transition_pose(owner, heading);
    reinterpret_cast<void (__fastcall *)(plr_loco_crawl_transition_state *, void *, actor *)>(
        get_vfunc(m_vtbl, 0x44))(this, nullptr, owner);
    hero->field_240 = true;
    auto *animation = static_cast<ai::als_inode *>(get_core()->get_info_node(ai::als_inode::default_id, true));
    if (controller)
        controller->frame_advance(g_world_ptr->time_manager.field_18);
    const auto transition = static_cast<int>(hero->field_20C.field_0);
    if (wallrun && owner->has_physical_ifc() && !(transition >= 1 && transition <= 5))
        var<vector3d>(0x00958328) = heading;
    animation->request_category_transition(transition_als_category_hash, static_cast<als::layer_types>(0), true, false, false);
    hero->update_wall_run_als_params();
    hero->update_crawl_als_params();
    hero->field_20C.update_crawl_transition_als_params(animation);
    animation->field_1C->force_update();
    hero->compute_curr_ground_plane(static_cast<force_recompute_enum>(1), 2.5f);
}

void plr_loco_crawl_transition_state::deactivate(const ai::mashed_state *state)
{
    if constexpr (!STANDALONE_SYSTEM) {
        THISCALL(0x0046A9A0, this, state);
        return;
    }
    auto *owner = get_actor();
    auto *hero = static_cast<ai::hero_inode *>(get_core()->get_info_node(ai::hero_inode::default_id, true));
    const auto transition = static_cast<int>(hero->field_20C.field_0);
    if (owner->get_abs_po().get_y_facing().y > 0.8f) {
        auto *animation = static_cast<ai::als_inode *>(get_core()->get_info_node(ai::als_inode::default_id, true));
        static const string_hash jump_air{to_hash("Jump_Air")};
        static const string_hash idle_walk_run{static_cast<int>(to_hash("Idle_Walk_Run"))};
        animation->request_category_transition(transition == 4 ? jump_air : idle_walk_run,
            static_cast<als::layer_types>(0), true, false, false);
        hero->compute_curr_ground_plane(static_cast<force_recompute_enum>(1), 2.5f);
        if (auto *controller = owner->m_player_controller)
            controller->frame_advance(g_world_ptr->time_manager.field_18);
        hero->update_crawl_als_params();
        animation->field_1C->force_update();
        if (owner->has_physical_ifc() && transition != 4)
            owner->physical_ifc()->manage_standing(true);
    }
    if (owner->has_physical_ifc())
        owner->physical_ifc()->field_C &= ~0x200u;
    if (auto *capsule = get_core()->field_70)
        capsule->set_mode(static_cast<capsule_alter_sys::eAlterMode>(3));
    if (owner->has_physical_ifc())
        owner->physical_ifc()->set_gravity(true);
    hero->field_240 = false;
    ai::base_state::_deactivate(state);
}

ai::state_trans_messages plr_loco_crawl_transition_state::frame_advance(Float dt)
{
    if constexpr (!STANDALONE_SYSTEM)
        return static_cast<ai::state_trans_messages>(THISCALL(0x0045AA40, this, dt));
    auto result = ai::enhanced_state::frame_advance(dt);
    auto *hero = static_cast<ai::hero_inode *>(get_core()->get_info_node(ai::hero_inode::default_id, true));
    get_core()->get_info_node(ai::als_inode::default_id, true);
    hero->update_crawl_als_params();
    auto *animation = static_cast<ai::als_inode *>(get_core()->get_info_node(ai::als_inode::default_id, true));
    const bool no_transition = static_cast<int>(hero->field_20C.field_0) == 0;
    if (animation->anim_finished(transition_als_category_hash, static_cast<als::layer_types>(0)) || no_transition) {
        auto *owner = get_actor();
        set_crawl_transition_pose(owner, owner->get_abs_po().get_y_facing());
        if (owner->get_abs_po().get_y_facing().y <= 0.8f &&
            (no_transition || hero->field_20C.field_10.y <= 0.732421875f)) {
            result = static_cast<ai::state_trans_messages>(73);
        } else if (static_cast<int>(hero->field_20C.field_0) == 4) {
            hero->set_jump_type(static_cast<ai::eJumpType>(0), false);
            result = static_cast<ai::state_trans_messages>(72);
        } else {
            result = static_cast<ai::state_trans_messages>(71);
        }
        if (no_transition) {
            string_hash terrain;
            owner->physical_ifc()->get_parent_terrain_type(&terrain);
            owner->my_sound_and_pfx_interface->play_terrain_sound(static_cast<eTerrainSoundType>(8), terrain, 1.0f);
        }
    }
    return result;
}

void plr_loco_crawl_transition_state_patch()
{
    {
        FUNC_ADDRESS(address, &plr_loco_crawl_transition_state::activate);
        set_vfunc(0x00875E30, address);
    }
    {
        FUNC_ADDRESS(address, &plr_loco_crawl_transition_state::deactivate);
        set_vfunc(0x00875E34, address);
    }
    {
        FUNC_ADDRESS(address, &plr_loco_crawl_transition_state::frame_advance);
        set_vfunc(0x00875E38, address);
    }
}
