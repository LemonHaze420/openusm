#pragma once

#include "rb_collision_capsule.h"
#include "vector3d.h"
#include "entity_base_vhandle.h"

struct rigid_body;
struct subdivision_node;

struct rb_collision_sphere {
    vector3d position;
    vector3d field_C;
    vector3d local_position;
    float radius;
    bool contact;
    char padding[3];
    entity_base_vhandle collision_entity;
    subdivision_node *collision_node;
};

struct alignas(16) rigid_body_sphere_list {
    rb_collision_sphere spheres[2];
    rb_collision_sphere *sphere_data;
    int sphere_count;
    rb_collision_sphere bounds;
    rb_collision_capsule capsule;
    float collision_scale;
    float smallest_radius;
    vector3d previous_position;
    vector3d field_EC;
    int tunnel_count;
    rigid_body *body;
    int body_index;

    void initialize();
    void set(rigid_body *rigid, float scale, const vector3d &position);
    void calc_bounding_sphere();
};

struct biped_sphere_pair {
    rigid_body_sphere_list *first;
    rigid_body_sphere_list *second;
};
