#pragma once

#include "variable.h"
#include "vector3d.h"

#include <vector.hpp>

struct region;
struct subdivision_node;
struct subdivision_visitor;

struct stack_allocator;
struct proximity_map_construction_leaf;

struct traverse_test {
    int field_0;
    vector3d field_4;
    float field_10;
};


struct region_mirror_data {
    region *field_0;
    vector3d field_4;
    vector3d field_10;

    region_mirror_data(region *);
};

struct static_region_list_methods {
    std::intptr_t m_vtbl;

    //0x00513F90
    static void init();

    //0x00513FF0
    static void term();

    //0x00513F40
    //virtual
    int traverse_point(const subdivision_node &a1, const vector3d &a2, subdivision_visitor &a3);

    //0x00514030
    //virtual
    int traverse_all(const subdivision_node &a2, subdivision_visitor &a3, bool);

    //0x00513EF0
    //virtual
    int traverse_sphere(const subdivision_node &a2, const vector3d &arg4, float a4, subdivision_visitor &a3);

    //0x00523900
    //virtual
    int traverse_line_segment(const subdivision_node &a2, const vector3d &, const vector3d &,
                              subdivision_visitor &visitor);

    //0x00513E50
    int traverse_using_test(const traverse_test &a1, const subdivision_node &a2, subdivision_visitor &a3);

    static inline Var<int *> scratchpad{0x0095C8B8};

    static inline Var<region_mirror_data *> mirror{0x0095C8BC};
};

struct subdivision_node_builder {
    int m_vtbl;
    int field_4;

    subdivision_node_builder();
};

struct static_region_list_builder : subdivision_node_builder {
    static_region_list_builder();

    void build_mirror(stack_allocator &a1, _std::vector<proximity_map_construction_leaf> &arg4);
};

extern void static_region_list_methods_patch();
