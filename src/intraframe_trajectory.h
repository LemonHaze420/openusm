#pragma once

#include "capsule.h"
#include "fixed_pool.h"
#include "oldmath_po.h"

struct sphere;
struct actor;
struct entity;
struct dynamic_conglomerate_clone;
struct fixed_pool;
struct collision_capsule;
struct collision_geometry;

struct collision_free_state {
    po xform;
    capsule rel_cap;
    bool field_5C;
    int field_60;
};

struct intraframe_trajectory_t {
    po world_po0;
    po world_po1;
    po final_po;
    capsule my_abs_cap0;
    capsule my_abs_cap1;
    capsule relcap0;
    capsule relcap1;
    capsule *final_relcap;
    bool is_capsule;
    actor *ent;
    dynamic_conglomerate_clone *field_13C;
    vector3d field_140;
    float field_14C;
    vector3d field_150;
    intraframe_trajectory_t *field_15C;
    int field_160;
    bool field_164;
    bool field_165;

    struct iterator {
        intraframe_trajectory_t *trj;
        iterator *field_4;
    };

    iterator *field_168;

    intraframe_trajectory_t();

    intraframe_trajectory_t(actor *a2, Float a3, const po &a4, dynamic_conglomerate_clone *a5);

    void init(entity *a2, Float a3, const po &a4, dynamic_conglomerate_clone *a5);

    //0x0053BB20
    void init_capsules();

    //0x0051BF00
    void integrate(Float a2, po *integrated_xform);

    //0x0051BFF0
    [[nodiscard]] capsule get_abs_cap0();

    //0x0051C010
    [[nodiscard]] capsule get_abs_cap1();

    //0x005282A0
    [[nodiscard]] sphere get_bounding_sphere();

    collision_capsule &get_capsule();

    collision_geometry *get_colgeom();

    bool has_colgeom()
    {
        return this->get_colgeom() != nullptr;
    }


    static inline Var<fixed_pool> pool{0x0092244C};
};
