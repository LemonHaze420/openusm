#pragma once

#include "config.h"

struct vector3d;

namespace als {

struct param {
    int field_0;
    float field_4;
};

struct param_node {
    param_node(param a2);

    param field_0;
    param_node *field_8;

    param_node *field_C;
};

struct param_list {
    param_node *field_0;

    //0x00493BB0
    param_list();

    //0x00499690
    void add_param(uint32_t a1, const vector3d &a2);

    //0x00499620
    void add_param(param a2);

    bool is_empty() const;

    void insert_node(param_node *a2);

    //0x0049FE70
    void concat_list(als::param_list &a2);

    //0x0049E1D0
    void cull_duplicates_keep_last();

    //0x0049E270
    void clear();
};
}  // namespace als
