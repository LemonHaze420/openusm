#pragma once

#include <cstdint>

#include "entity_base_vhandle.h"
#include "ngl_math.h"
#include "vector3d.h"

struct rtree_root_t;
struct subdivision_visitor;

//0x00520FA0
extern void traverse_rtree(const vector3d &a1, const vector3d &a2, const rtree_root_t &a3, subdivision_visitor &a4);

struct rtree_construction_node_t {
    vector3d field_0;
    vector3d field_C;
    entity_base_vhandle field_18;

    rtree_construction_node_t(entity_base_vhandle a2, const vector3d &a1, const vector3d &a4);
};

struct rtree_node_t {
    int16_t minx;
    int16_t maxx;
    int16_t miny;
    int16_t maxy;
    int16_t minz;
    int16_t maxz;
    entity_base_vhandle field_C;

    //0x0055E9F0
    void init(const rtree_construction_node_t &a2, const math::VecClass<4, -1> &a3, const math::VecClass<4, -1> &a4);

    void clear();

    bool is_valid() const
    {
        return this->field_C.field_0 != 0xFFFFFFFF;
    }
};
