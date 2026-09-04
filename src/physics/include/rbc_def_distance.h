#pragma once

#include "rbc_base.h"
#include "float.hpp"
#include "pulse_sum_cache.h"
#include "phys_vector3d.h"

struct phys_vector3d;
struct outer_time;
struct physics_system;

struct rigid_body_constraint_distance : rigid_body_constraint {
    rigid_body_constraint_distance *field_8;
    phys_vector3d field_C;
    int field_18;
    phys_vector3d field_1C;
    int field_28;
    float m_min_distance;
    float m_max_distance;
    float field_34;
    float field_38;
    float field_3C;
    int field_40;
    pulse_sum_cache field_44[3];

    void set(phys_vector3d const &a2, phys_vector3d const &a3, Float a4, Float a5);

    void outer_epilog_update(const outer_time &a2);

    void outer_prolog_update(const outer_time &a2);

    void setup_constraint(physics_system *a2, Float a3);

    void sub_502680(const vector3d &a2);
};
