#pragma once

struct entity_base;
struct conglomerate;
struct vector3d;
struct po;

#include "oldmath_po.h"
#include "quaternion.h"
struct phys_anim_bone_entry {
    entity_base *bone;
    int rigid_body_index;
    int field_8;
    int saved_pose_index;
};

struct phys_anim_saved_pose {
    po relative;
    quaternion original_rotation;
    quaternion physics_rotation;
};

struct phys_anim_bone_array {
    char field_0[0x5A0];
    phys_anim_bone_entry *bones;
    int bone_count;
    char field_5A8[0xB48];
    phys_anim_saved_pose *saved_poses;
    int field_10F4;
    int field_10F8;
    int field_10FC;
    conglomerate *owner;

    void attach_physics_bones();
    void save_poses(po *absolute, po *relative);
    void restore_poses(const po *absolute, const po *relative);
    void prepare_physics_pose();
    void copy_back_tween_recurse(float fraction);
    void copy_back_bones_recurse(entity_base *parent = nullptr);
    void update_owner_matrix(const vector3d &position);

private:
    void copy_back_bones(entity_base *parent, int &index);
    void copy_bones(entity_base *parent);
    void assign_parents(entity_base *parent, int rigid_parent, int &index);
    bool assign_saved_poses(entity_base *parent, int &index);
    void copy_back_tween(entity_base *parent, int &index, float fraction);
};
