#pragma once

#include "vector3d.h"
#include "pulse_sum_cache.h"

struct physics_system;
struct rigid_body;
struct phys_vector3d;
struct rigid_body_constraint_contact;
struct rigid_body_constraint_distance;
struct rigid_body_constraint_ragdoll;

struct physics_vec4 { float x, y, z, w; };
struct physics_constraint_link {
    rigid_body *b1;
    rigid_body *b2;
    physics_constraint_link *next;
};


struct rb_partition_node {
    physics_constraint_link *constraints[7];
    rigid_body *body;
    rb_partition_node *head;
    rb_partition_node *tail;
    int body_count;
    int steps;
    float elapsed;
    rb_partition_node *next;
};


struct physics_pulse_body {
    physics_pulse_body *next;
    physics_vec4 velocity;
    physics_vec4 angular_velocity;
    float inverse_mass;
    rigid_body *body;
};

struct physics_pulse_scalar {
    physics_pulse_scalar *next;
    physics_vec4 direction;
    physics_vec4 anchor1;
    physics_vec4 anchor2;
    physics_vec4 angular1;
    physics_vec4 angular2;
    float lower;
    float upper;
    float impulse;
    float previous_impulse;
    float residual;
    float previous_residual;
    float position_target;
    float velocity_target;
    float softness;
    float denominator;
    float friction;
    unsigned flags;
    physics_pulse_scalar *normal;
    physics_pulse_body *body1;
    physics_pulse_body *body2;
    pulse_sum_cache *cache;
};

struct physics_pulse_angular {
    physics_pulse_angular *next;
    physics_vec4 direction;
    physics_vec4 anchor1;
    physics_vec4 anchor2;
    physics_vec4 angular1;
    physics_vec4 angular2;
    float lower, upper, impulse, previous_impulse, residual, previous_residual;
    float position_target, velocity_target, softness, denominator;
    unsigned flags;
    physics_pulse_body *body1;
    physics_pulse_body *body2;
    pulse_sum_cache *cache;
};

struct physics_pulse_point {
    physics_pulse_point *next;
    physics_vec4 anchor1, anchor2;
    physics_vec4 angular[6];
    physics_vec4 impulse, previous_impulse, residual, previous_residual;
    physics_vec4 velocity_target, position_target;
    physics_vec4 inverse[3];
    physics_vec4 diagonal;
    physics_pulse_body *body1;
    physics_pulse_body *body2;
    pulse_sum_cache *cache;
};


rb_partition_node *physics_build_constraint_partitions(physics_system *world);
void physics_execute_constraint_solver(physics_system *world, rb_partition_node *head, int visit, int next_visit);

rigid_body_constraint_contact **physics_contact_insert(physics_system *world, rigid_body *b1, rigid_body *b2,
    rigid_body_constraint_contact *contact);
rigid_body_constraint_contact *physics_find_contact(physics_system *world, rigid_body *b1, rigid_body *b2);

void physics_setup_ragdoll(rigid_body_constraint_ragdoll *joint, physics_system *world, float elapsed);
void physics_setup_distance(rigid_body_constraint_distance *joint, physics_system *world, float elapsed);
void physics_setup_contact(rigid_body_constraint_contact *contact, physics_system *world, float elapsed);
void physics_add_contact(rigid_body_constraint_contact *contact, rigid_body *first, rigid_body *second,
    const phys_vector3d &point1, const phys_vector3d &point2, const phys_vector3d &normal,
    float friction, float bounce, float maximum_bounce, bool);
void physics_set_scalar(physics_pulse_scalar *pulse, rigid_body *first, physics_vec4 anchor1,
    rigid_body *second, physics_vec4 anchor2, physics_vec4 direction, pulse_sum_cache *cache,
    physics_vec4 offset);
