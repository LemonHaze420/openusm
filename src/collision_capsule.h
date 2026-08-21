#pragma once

#include "capsule.h"
#include "collision_geometry.h"

struct actor;

struct collision_capsule : collision_geometry {
    capsule rel_cap;

    collision_capsule();

    //0x00524CD0
    collision_capsule(actor *a2);

    capsule get_abs_capsule(const po &a3);

    //0x00515F20
    void compute_dimensions();

    void validate();

    /* virtual */ ~collision_capsule() = default;

    /* virtual */ collision_geometry *make_instance(actor *a2) /* override */;

    //0x005160C0
    /* virtual */ vector3d get_local_space_bounding_sphere_center() /* override */;

    //0x00516080
    /* virtual */ float get_bounding_sphere_radius() /* override */;

    //0x00502020
    /* virtual */ float get_core_radius() /* override */;

    /* virtual */ int get_type() /* override */;

    //0x00524DE0
    /* virtual */ void un_mash(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3) /* override */;

    /* virtual */ void render(const po &a1);
};

extern std::intptr_t &collision_capsule_v_table;

extern void collision_capsule_patch();
