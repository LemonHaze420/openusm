#pragma once

#include "rb_collision_capsule.h"

struct rigid_body;

struct rigid_body_sphere_list {
    char field_0[0xA4];
    rb_collision_capsule field_A4;
    int field_D8[9];
    rigid_body *field_FC;
};
