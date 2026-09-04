#pragma once

#include "collision_geometry.h"

#include "colmesh_common.h"
#include "variable.h"

#include <cstdint>

struct actor;

struct collision_mesh {
    char field_0[4];
    uint32_t m_version;
    int field_8;
    int field_C;
    collision_obb_t field_10[1];
};

struct cg_mesh : collision_geometry {
    collision_mesh *data;
    int field_14;

    //0x005195E0
    cg_mesh();

    /* virtual */ ~cg_mesh() = default;

    //0x005267F0
    /* virtual */ collision_geometry *make_instance(actor *a2) /* override */;

    /* virtual */ vector3d get_local_space_bounding_sphere_center() /* override */;

    /* virtual */ float get_bounding_sphere_radius() /* override */;

    /* virtual */ int get_type() /* override */;

    //0x0053B100
    /* virtual */ void _un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4) /* override */;
};

extern Var<std::intptr_t> collision_mesh_v_table;

extern void cg_mesh_patch();
