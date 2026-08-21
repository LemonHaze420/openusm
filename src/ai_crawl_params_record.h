#pragma once

#include "vector3d.h"

enum crawl_transition_type_enum {};

namespace ai {

struct als_inode;

struct crawl_params_record {
    crawl_transition_type_enum field_0;
    bool field_4;
    bool field_5;
    float field_8;
    float field_C;
    vector3d field_10;
    vector3d field_1C;
    float field_28;

    void update_crawl_transition_als_params(ai::als_inode *a2);
};

}  // namespace ai
