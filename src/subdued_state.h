#pragma once

#include "ai_common_states.h"

namespace ai {

struct subdued_state : std_interrupt_state {
    bool category_requested;
    unsigned char padding_31[3];
    float hold_time;
    float field_38;
    unsigned field_3C;
    float webbing;

    subdued_state();
    explicit subdued_state(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                   const param_block *params, activate_flag_e flags);
    void _deactivate(const mashed_state *next);
    state_trans_messages _frame_advance(Float delta);
    state_trans_action _check_transition(Float delta);

    static inline string_hash default_id{int(to_hash("subdued"))};
};

}  // namespace ai
