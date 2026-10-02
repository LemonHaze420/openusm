#pragma once

#include "als_transition_post_handle.h"
#include "scripted_trans_group.h"
#include "string_hash.h"

namespace als {
struct request_data {
    bool did_transition_occur;
    bool do_post_action;
    bool is_trans_to_category;
    bool ignore_no_transition;
    bool post_req_for_category;
    string_hash field_8;
    transition_post_handle field_C;

    request_data();

    request_data(const request_data &);

    void clear();

    //0x004AD310
    bool did_rule_pass() const
    {
        return this->did_transition_occur || this->do_post_action;
    }

    //0x004ADF40
    void operator=(const request_data &a2);
};

}  // namespace als
