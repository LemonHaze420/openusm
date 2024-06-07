#pragma once

struct generic_mash_header;
struct generic_mash_data_ptrs;
struct resource_key;

struct city_gradients {
    int field_0;
    int field_4;
    int num_gradients;
    int num_gradient_keys;
    void *field_10;
    void *field_14;
    resource_key *field_18;

    //0x00527690 
    void un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *a5);

    //0x0051ACD0
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *a4);
};
