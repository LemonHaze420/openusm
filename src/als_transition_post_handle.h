#pragma once

#include "scripted_trans_group.h"

namespace als {

struct basic_rule_data;

struct transition_post_handle {
    basic_rule_data *field_0;
    scripted_trans_group::transition_type field_4;
};
}  // namespace als
