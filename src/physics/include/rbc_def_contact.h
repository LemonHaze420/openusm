#pragma once

#include "float.hpp"
#include "pulse_sum_cache.h"

struct rigid_body;
struct phys_vector3d;
struct physics_system;

struct contact_point_info {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    float m_fric_coef;
    float m_bounce_coef;
    int field_4C;
    bool field_50;
    pulse_sum_cache field_54[3];
};

struct rigid_body_constraint_contact {
    rigid_body *b1;
    rigid_body *b2;
    int field_8;
    int filed_C;
    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;

    auto get_b1()
    {
        return b1;
    }

    auto get_b2()
    {
        return b2;
    }

    //0x007A7360
    void add_point(rigid_body *, rigid_body *, phys_vector3d const &, phys_vector3d const &, phys_vector3d const &,
                   Float fric_coef, Float bounce_coef, Float, bool);

    void setup_constraint(physics_system *a2, Float a3);
};


extern void rbc_contact_patch();
