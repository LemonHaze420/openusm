#pragma once

#include "float.hpp"

struct mString;
struct nglMatrix;
struct generic_mash_header;
struct generic_mash_data_ptrs;

struct city_lights {
    char *field_0;
    char *field_4;
    char *field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;

    //0x0051B5C0
    void load_from_file(const mString &a2);

    //0x00527EB0
    void update(Float a1, nglMatrix &a2, nglMatrix &a3);

    //0x00527E90
    void un_mash_start(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3, void *a4);

    //0x0051B520
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *a4);
};

extern void city_lights_patch();
