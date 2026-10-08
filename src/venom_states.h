#pragma once

#include "ai_std_jump_state.h"

namespace ai {

struct retaliation_state : enhanced_state {
    explicit retaliation_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _get_info_node_list(info_node_desc_list &list);
    state_trans_action _check_transition(Float dt);
};

struct venom_combat_idle_state : enhanced_state {
    explicit venom_combat_idle_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _deactivate(const mashed_state *next);
    state_trans_messages _frame_advance(Float dt);
    void _get_info_node_list(info_node_desc_list &list);
    state_trans_action _check_transition(Float dt);
};

struct venom_feed_check_state : enhanced_state {
    explicit venom_feed_check_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _get_info_node_list(info_node_desc_list &list);
    state_trans_action _check_transition(Float dt);
};

struct venom_phase_check_state : enhanced_state {
    explicit venom_phase_check_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _get_info_node_list(info_node_desc_list &list);
    state_trans_action _check_transition(Float dt);
};

struct venom_jump_attack_state : ai_std_jump_state {
    entity_base_vhandle target;
    float saved_gravity_multiplier;

    explicit venom_jump_attack_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _deactivate(const mashed_state *next);
    void _get_info_node_list(info_node_desc_list &list);
    void setup_jump(ai_state_machine *machine);
};

struct venom_jump_chase_state : venom_jump_attack_state {
    explicit venom_jump_chase_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    state_trans_action _check_transition(Float dt);
    void setup_jump(ai_state_machine *machine);
    void update_target_prediction();
};

}
