#pragma once

#include "ai_common_states.h"
#include "combo_system_move.h"

namespace ai {

struct hit_react_state : std_interrupt_state {
    bool category_entered;
    unsigned char padding_31[3];
    string_hash category;
    int attack_id;
    combat_inode *combat;
    als_inode *animation;
    entity_base_vhandle attacker;
    bool web_reaction;
    bool was_webbed;
    bool transition_pending;
    bool field_4B;
    string_hash next_state;
    combo_system_move::results reaction;
    vector3d direction;
    bool field_D8;
    unsigned char padding_D9[3];

    hit_react_state();
    explicit hit_react_state(from_mash_in_place_constructor *tag);
    ~hit_react_state();
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _deactivate(const mashed_state *next);
    state_trans_messages _frame_advance(Float delta);
    state_trans_action _check_transition(Float delta);
    bool is_done_hit_reacting();
    bool is_event_raised(Float delta);
    bool resist_feeding();
    bool pending_attack(combo_system_move::results &out);
    bool forced_response(combo_system_move::results &out, vector3d &out_direction);
    void update_source_parameters();

    static const inline string_hash default_id{to_hash("hit_react")};
};

}  // namespace ai
