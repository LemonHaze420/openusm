#pragma once

#include "vector3d.h"

struct entity_base;
struct conglomerate;

struct bone_mass_info {
    int start_bone;
    int end_bone;
    float center_fraction;
    vector3d end_offset;
    bool offset_in_end_frame;
    char padding[3];
    float start_trim;
    float end_trim;
    int sphere_count;
    vector3d start;
    vector3d end;
    vector3d local_center;
    vector3d joint_world1;
    vector3d joint_world2;
    float joint_damping;
    vector3d joint_offset;
    float mass;
    vector3d inertia;
    float radius;
    float collision_scale;
    float field_8C;
    int body_index;
    int parent_index;
    entity_base *bone;
    entity_base *parent;
    int joint_type;
    float minimum_angle;
    float maximum_angle;
    vector3d anchor1;
    vector3d anchor2;
    vector3d axis1;
    vector3d axis2;
    vector3d reference1;
    vector3d reference2;
    int limit_count;
    vector3d limit_axes[4];
    float limit_angles[4];

    void calc_stuff(conglomerate *owner);
};

const char *biped_bone_name(int index);
void setup_bone_mass_info(bone_mass_info (&bones)[10], conglomerate *owner, int type);
