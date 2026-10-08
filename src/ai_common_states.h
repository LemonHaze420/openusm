#pragma once

#include "enhanced_state.h"
#include "vector3d.h"
#include "entity_base_vhandle.h"

namespace ai {
struct combat_inode;
struct als_inode;

struct std_interrupt_state : enhanced_state {
    std_interrupt_state();
    explicit std_interrupt_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    state_trans_action interrupt_message(Float delta, state_trans_messages message) const;
    state_trans_action interrupt_default() const;
};

struct generic_target_hero_state : enhanced_state {
    bool failed;
    unsigned char padding[3];
    generic_target_hero_state();
    explicit generic_target_hero_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    state_trans_action _check_transition(Float delta);
};

struct prepare_combo_state : enhanced_state {
    prepare_combo_state();
    explicit prepare_combo_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    state_trans_messages _frame_advance(Float delta);
    string_hash get_combo_to_use() const;
};

struct falling_state : std_interrupt_state {
    unsigned fields_30[2];
    falling_state();
    explicit falling_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    state_trans_messages _frame_advance(Float delta);
};

struct unconscious_state : std_interrupt_state {
    float unconscious_time;
    int phase;
    unconscious_state();
    explicit unconscious_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    state_trans_messages _frame_advance(Float delta);
    state_trans_action _check_transition(Float delta);
};

struct hit_avoid_state : std_interrupt_state {
    bool category_entered;
    unsigned char padding_31[3];
    string_hash category;
    unsigned field_38;
    combat_inode *combat;
    als_inode *animation;
    entity_base_vhandle attacker;
    vector3d direction;

    hit_avoid_state();
    explicit hit_avoid_state(from_mash_in_place_constructor *tag);
    ~hit_avoid_state();
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _deactivate(const mashed_state *next);
    state_trans_messages _frame_advance(Float delta);
    bool finished();
};

}
