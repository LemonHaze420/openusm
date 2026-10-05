#pragma once

#include "enhanced_state.h"

namespace ai {

struct signal_enhanced_state : enhanced_state {
    bool field_30;

    signal_enhanced_state();
    static void *native_vtable();
    void activate(ai_state_machine *machine, const mashed_state *state, const mashed_state *previous,
                  const param_block *params, activate_flag_e flags);
    state_trans_action check_transition(Float time);

    signal_enhanced_state(from_mash_in_place_constructor *a2);
};
}  // namespace ai
