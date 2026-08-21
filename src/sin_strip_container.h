#pragma once

#include "mashable_vector.h"

struct sin_district_container;

struct generic_mash_header;
struct generic_mash_data_ptrs;

struct sin_strip_container {
    char *strip_name;
    mashable_vector<sin_district_container> field_4;
    int field_C;

    sin_strip_container();

    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *a4);
};
