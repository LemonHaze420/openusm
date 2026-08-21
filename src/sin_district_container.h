#pragma once

struct generic_mash_header;
struct generic_mash_data_ptrs;

struct sin_district_container {
    char *district_name;
    int field_0[7];

    sin_district_container();

    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);
};
