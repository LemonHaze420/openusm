#pragma once

#include "variable.h"
#include "subdivision_node.h"
#include "float.hpp"
#include "vector3d.h"

struct fixed_pool;
struct subdivision_visitor;
struct sector2d;
struct dynamic_proximity_map_stack;

inline constexpr auto STOP_VISITING_FARTHER_CELLS = 2;
inline constexpr auto STOP_VISITING_WITHIN_THIS_CELL = 3;

struct cell_index {
    int16_t x, y;
};

struct proximity_map {
    enum proximity_map_type_t { DYNAMIC = 1 };

    unsigned short field_0;
    proximity_map_type_t map_type;
    unsigned short field_8;
    vector3d field_C;
    vector3d field_18;
    vector3d field_24;
    vector3d field_30;
    vector3d field_3C;
    vector3d field_48;
    int number_of_cells;
    int log_number_of_cells;
    bool initialized;

    void map_vector3d_range_to_cell_range_with_swapping(const vector3d &a2, const vector3d &a3, cell_index &start_index,
                                                        cell_index &end_index);

    void map_vector3d_to_cell_index(const vector3d &a2, cell_index *a3);

    //0x005115E0
    int traverse_point(const vector3d &a2, subdivision_visitor &a3);

    //0x005229B0
    int traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4);

    //0x00522680
    void traverse_sector_raster(const sector2d &sec, Float sector_radius, subdivision_visitor &visitor);

    bool is_initialized();
};

struct dynamic_proximity_map : proximity_map {
    //0x00522500
    void init(dynamic_proximity_map_stack *alloc, fixed_pool *a3, int number_of_cells_arg, const vector3d &a5,
              const vector3d &a6, subdivision_node::type_t a7);
};

struct region;

struct proximity_map_construction_leaf {
    union {
        int i;
        region *r;
    } field_0;
    vector3d field_4;
    vector3d field_10;

    proximity_map_construction_leaf(region *a2, const vector3d &a1, const vector3d &a4);
};
