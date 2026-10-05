#pragma once

#include "game_camera.h"

struct string_hash;

struct marky_camera : game_camera {
    vector3d field_1A0;
    vector3d field_1AC;
    float field_1B8;
    bool field_1BC;
    bool field_1BD;
    vector3d field_1C0;
    vector3d field_1CC;
    float field_1D8;
    float field_1DC;

    //0x0057D7F0
    marky_camera(const string_hash &a2);

    void *operator new(size_t size);
    void operator delete(void *ptr);
    static void *native_vtable();
    void frame_advance(Float dt);
    void make_po();
    void camera_set_target(const vector3d &target);
    vector3d camera_get_target() const;
    void camera_set_roll(float roll);
    bool camera_slide_to(const vector3d &position, const vector3d &target, float roll, float speed);
    bool camera_slide_to_orbit(const vector3d &center, float radius, float azimuth, float elevation, float speed);
    void camera_orbit(const vector3d &center, float radius, float azimuth, float elevation);

    //0x00578080
    void set_affixed_x_facing(bool a2);

    //0x0057F350
    //virtual
    void sync(camera &a2);

    //0x00581220
    /* virtual */ void camera_set_collide_with_world(bool a2);
};
