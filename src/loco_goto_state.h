#pragma once

#include "enhanced_state.h"
#include "vector3d.h"

struct ai_path;
namespace ai {
struct loco_inode;
struct avoidance_inode;
struct loco_goto_state : enhanced_state {
    loco_inode *locomotion;
    avoidance_inode *avoidance;
    loco_goto_state();
    explicit loco_goto_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *, const mashed_state *, const mashed_state *, const param_block *,
                   activate_flag_e);
    state_trans_messages _frame_advance(Float delta);
    loco_inode *select_inode() const;
    vector3d steer_direction(const vector3d &, Float);
    void update_animation(const vector3d &, float);
    bool is_at_destination(float distance_squared, bool last);
    bool is_stuck(Float)
    {
        return false;
    }
    state_trans_messages do_checks(const vector3d &, float, Float);
    const vector3d &get_destination() const;
    bool is_last_destination() const
    {
        return true;
    }
    void repath();
    void update_category();
};
struct nonpathed_goto_state : loco_goto_state {
    float stuck_time;
    vector3d last_position;
    nonpathed_goto_state();
    explicit nonpathed_goto_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *, const mashed_state *, const mashed_state *, const param_block *,
                   activate_flag_e);
    loco_inode *select_inode() const;
    bool is_stuck(Float delta);
};
struct pathed_goto_state : nonpathed_goto_state {
    ai_path *path;
    pathed_goto_state();
    explicit pathed_goto_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *, const mashed_state *, const mashed_state *, const param_block *,
                   activate_flag_e);
    void _deactivate(const mashed_state *);
    state_trans_messages _frame_advance(Float delta);
    state_trans_action _check_transition(Float) const;
    state_trans_action _process_message(Float, state_trans_messages);
    state_trans_messages do_checks(const vector3d &, float, Float);
    const vector3d &get_destination() const;
    bool is_last_destination() const;
    void repath();
};
}
