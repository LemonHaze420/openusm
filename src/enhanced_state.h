#pragma once

#include "base_state.h"

#include "state_trans_action.h"
#include "float.hpp"
#include "state_trans_messages.h"
#include "string_hash.h"
#include "variable.h"

struct from_mash_in_place_constructor;

namespace ai {
struct enhanced_state : base_state {
    float field_1C;
    state_trans_messages field_20;
    int field_24;
    bool field_28;
    state_trans_actions field_2C;

    enhanced_state();

    //0x006BD970
    enhanced_state(from_mash_in_place_constructor *);

    float get_timeout_timer();

    bool is_default_transition_state() const
    {
        return this->my_mashed_state == nullptr;
    }

    bool can_handle_message(ai::state_trans_messages a2, bool a3) const;

    //0x006BD5A0
    /* virtual */
    state_trans_messages frame_advance(Float dt);

    //0x006CE1D0
    /* virtual */ void activate(ai_state_machine *a2, const mashed_state *a3, const mashed_state *a4,
                                const param_block *a5, activate_flag_e a6) /* override */;

    //0x006BD600
    /* virtual */ state_trans_action check_transition(Float a3) /* override */;

    //0x006D34F0
    /* virtual */ state_trans_action process_message(Float a3, state_trans_messages a4) const;

    //0x0043A040
    //virtual
    [[nodiscard]] state_trans_action get_default_return_code() const;

    /* virtual */ state_trans_action process_exit_message(Float a3, state_trans_messages the_msg) const;

    state_trans_action state_exit(string_hash a3, string_hash a4, state_trans_messages a5, state_trans_action a6) const;

    ai::state_trans_action exit_layer(string_hash a3, state_trans_messages a4, state_trans_action a5) const;

    static inline Var<string_hash[1]> timeout_hashes{0x0096CD28};

    static inline Var<string_hash[1]> to_state_hashes{0x0096CEB0};

    static const inline string_hash to_state_always_hash{int(to_hash("to_state_always"))};

    static inline Var<string_hash[1]> exit_layer_hashes{0x0096D0C0};

    static const inline string_hash process_default_trans_hash{int(to_hash("process_default_trans"))};

    static const inline string_hash exit_layer_always_hash{int(to_hash("exit_layer_always"))};
};

}  // namespace ai


extern void enhanced_state_patch();
