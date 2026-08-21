#pragma once

#include "state_trans_messages.h"
#include "string_hash.h"

namespace ai {
struct param_block;

enum state_trans_actions {
    TRANSITION = 0,
    RETURN = 1,
    MACHINE_EXIT = 2,
};

struct state_trans_action {
    state_trans_actions the_action;
    string_hash field_4;
    state_trans_messages the_message;
    param_block *field_C;

    state_trans_action() = default;

    state_trans_action(state_trans_actions a1, string_hash a2, state_trans_messages a3, param_block *a4);

    //0x00401980
    state_trans_action(const state_trans_action &a2);

    state_trans_action &operator=(const state_trans_action &a2) = default;

    bool is_default() const
    {
        return this->the_action == 3 || this->the_action == 4;
    }
};

}  // namespace ai
