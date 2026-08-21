#include "state_trans_action.h"

#include "common.h"
#include "state_trans_messages.h"

#include <cassert>

namespace ai {
VALIDATE_SIZE(state_trans_action, 0x10);

state_trans_action::state_trans_action(state_trans_actions action, string_hash a2, state_trans_messages message,
                                       param_block *a4)
    : the_action(action), field_4(a2), the_message(message), field_C(a4)
{
    assert(the_message != TRANS_TOTAL_MSGS || the_action != MACHINE_EXIT);
}

state_trans_action::state_trans_action(const state_trans_action &a2)
{
    this->the_action = a2.the_action;
    this->field_4 = a2.field_4;
    this->the_message = a2.the_message;
    this->field_C = a2.field_C;
}
}  // namespace ai
