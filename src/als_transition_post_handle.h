#pragma once

#include "scripted_trans_group.h"
#include "als_transition_rule.h"

namespace als {

struct basic_rule_data;

struct transition_post_handle {
    union {
        void *field_0;
        implicit_transition_rule **implicit_rule;
        explicit_transition_rule **explicit_rule;
        incoming_transition_rule **incoming_rule;
    };
    scripted_trans_group::transition_type field_4;

    basic_rule_data &rule_data() const
    {
        return *static_cast<basic_rule_data *>(field_0);
    }
};
}  // namespace als
