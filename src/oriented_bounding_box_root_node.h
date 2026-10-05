#pragma once

#include "color32.h"
#include "float.hpp"
#include "rtree_root.h"

struct vector3d;
struct subdivision_visitor;
struct proximity_map;
struct region;

struct oriented_bounding_box_root_node : rtree_root_t {
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;
    int field_58;
    proximity_map *field_5C;
    int field_60;
    int field_64;
    int field_68;
    int field_6C;

    oriented_bounding_box_root_node();

    void set_color(color32 a2);

    //0x00522E50
    void traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4);

    void un_mash(char *a2, int *image_size_used, region *reg);
};
