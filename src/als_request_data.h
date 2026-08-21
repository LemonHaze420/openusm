#pragma once

#include "scripted_trans_group.h"
#include "string_hash.h"

namespace als {
struct request_data {
    bool did_transition_occur;
    bool field_1;
    bool field_2;
    bool field_3;
    bool field_4;
    string_hash field_8;
    int field_C;
    scripted_trans_group::transition_type field_10;

    request_data();

    request_data(const request_data &);

    void clear();

    void operator=(const request_data &a2);
};

}  // namespace als
