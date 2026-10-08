#pragma once

#include "enhanced_state.h"
#include "entity_base_vhandle.h"

struct entity_base;

namespace ai {

struct ai_std_jump_state : enhanced_state {
    int jump_phase;
    float phase_time;
    int anim_callback;
    entity_base_vhandle attack_target;
    entity_base *attack_bone;
    int attack_begin_callback;
    int attack_end_callback;
    string_hash jump_category;
    bool allow_early_exit;
    bool attack_applied;
    bool saved_default_transition;
    uint8_t field_53;
    float saved_physical_parameter;

    explicit ai_std_jump_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _deactivate(const mashed_state *next);
    state_trans_messages _frame_advance(Float dt);
    state_trans_action _check_transition(Float dt);
    void _get_info_node_list(info_node_desc_list &list);
    void set_jump_phase(int phase);
    void on_animation_action();
    void apply_jump_damage();
    void update_jump_parameters();
};

}
