#pragma once

#include "game_camera.h"

struct string_hash;

struct marky_camera : game_camera {
    vector3d field_1A0;
    vector3d field_1AC;
    int field_1B8;
    bool field_1BC;
    bool field_1BD;
    vector3d field_1C0;
    int empty0[3];
    float field_1D8;
    float field_1DC;

    //0x0057D7F0
    marky_camera(const string_hash &a2);

    void *operator new(size_t size);

    //0x00578080
    void set_affixed_x_facing(bool a2);

    //0x0057F350
    //virtual
    void sync(camera &a2);

    //0x00581220
    /* virtual */ void camera_set_collide_with_world(bool a2);
};
