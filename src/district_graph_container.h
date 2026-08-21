#pragma once

#include "mashable_vector.h"

struct terrain;

struct dsg_region_container {
    char *field_0;
    int field_4[17];

    mashable_vector<int> field_48;

    //0x00520B70
    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);
};

struct dsg_box_container {
    int field_0[13];

    //0x005105C0
    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);
};

struct district_graph_container {
    mashable_vector<dsg_region_container> field_0;
    mashable_vector<dsg_box_container> field_8;
    int field_10;

    //0x00556640
    void setup_terrain(terrain *the_terrain);

    void un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *a5);

    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);
};
