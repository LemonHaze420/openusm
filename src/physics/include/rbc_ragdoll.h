#pragma once

#include "rigid_body.h"
#include "vector3d.h"
#include <cstdint>

struct ragdoll_joint_limit {
    vector4d axis;
    float cosine;
    float sine;
};


struct rigid_body_constraint_ragdoll {
    rigid_body *b1;
    rigid_body *b2;
    rigid_body_constraint_ragdoll *next;
    vector4d anchor1;
    vector4d anchor2;
    std::uint32_t flags;
    int caches[20];
    vector4d axis1;
    vector4d axis2;
    vector4d tangent;
    vector4d bitangent;
    vector4d reference;
    vector4d minimum;
    vector4d maximum;
    ragdoll_joint_limit limits[2];
    ragdoll_joint_limit *limit_data;
    int limit_count;
    int field_128;
    int field_12C;
    float damping;
    char alignment_padding[12];

    void reset();
    void set(const vector3d &first, const vector3d &second);
    void set_damp_k(float value);
    void set_hinge(const vector3d &first, const vector3d &second, const vector3d &reference1,
                   const vector3d &reference2, float minimum_angle, float maximum_angle);
    void set_swivel(const vector3d &first, const vector3d &second, const vector3d &reference1,
                    const vector3d &reference2, float minimum_angle, float maximum_angle);
    void add_joint_limit(const vector3d &axis, float angle);
    float relax();
};
