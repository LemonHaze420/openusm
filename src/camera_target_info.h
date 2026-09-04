#pragma once

#include "ai_player_controller.h"
#include "float.hpp"
#include "vector3d.h"

struct entity;
struct actor;

struct camera_target_info {
    vector3d pos;
    vector3d field_C;
    float radius;
    float min_look_dist;
    float max_look_dist;
    vector3d field_24;
    vector3d up;
    vector3d facing;
    vector3d field_48;
    actor *field_54;
    int field_58;

    //0x004B3DC0
    camera_target_info(entity *target, Float a3, const vector3d &a4, const vector3d &a5);

    //0x004B28A0
    int get_loco_mode() const;

    eHeroLocoMode get_prev_loco_mode() const;

    float sub_4B42E0() const;

    bool sub_4B2980() const;

    bool sub_4B29C0() const;
};

extern float &g_camera_min_dist;

extern float &g_camera_max_dist;

extern float &g_camera_supermax_dist;

extern void camera_target_info_patch();
