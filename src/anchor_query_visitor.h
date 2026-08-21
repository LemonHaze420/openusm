#pragma once

#include "fixed_vector.h"
#include "quick_anchor_info.h"
#include "subdivision_visitor.h"
#include "vector3d.h"

struct subdivision_node;
struct conglomerate;
struct occupancy_voxels_t;
struct entity;
struct entity_base;
struct quick_anchor_container_t;

struct anchor_query_visitor : subdivision_visitor {
    quick_anchor_container_t *field_4;
    vector3d field_8;
    vector3d field_14;
    bool field_20;
    occupancy_voxels_t *field_24;

    anchor_query_visitor(quick_anchor_container_t *a2, const vector3d &a1, const vector3d &a4, bool a5,
                         occupancy_voxels_t *a6);

    //0x004901E0
    void add_quick_anchor(entity_base *anchor, entity *clone_ptr);

    //0x004860D0
    /* virtual */ int visit(subdivision_node *a2);
};

extern void anchor_query_visitor_patch();
