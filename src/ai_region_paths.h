#pragma once

#include "float.hpp"

struct region;
struct ai_quad_path;
struct vector3d;
struct ai_quad_path_cell;

struct ai_region_paths {
    char id[8];

    region *field_8;
    int field_C;
    ai_quad_path *quad_path_table;
    int quad_path_table_count;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;
    char *field_3C;

    ai_region_paths();

    //0x0046EFF0
    ai_quad_path *get_quad_path_for_point(const vector3d &a2, Float a3, ai_quad_path_cell **a4, bool a5,
                                          ai_quad_path *a6);

    ai_quad_path *get_quad_path_internal();

    ai_quad_path *get_quad_path(int index);

    //0x0046F0F0
    void un_mash(void *buffer_ptr, int *a3, region *reg);
};
