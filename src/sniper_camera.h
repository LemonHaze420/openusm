#pragma once

#include "game_camera.h"

struct sniper_camera : game_camera {
    int field_1A0;
    int field_1A4;
    float field_1A8;
    float field_1AC;
    float field_1B0;
    float field_1B4;

    //0x0057D8A0
    sniper_camera(const string_hash &a2, entity *a3);

    void *operator new(size_t size);
    void operator delete(void *ptr);
    static void *native_vtable();
    void sync(camera &source);
    void frame_advance(Float dt);
    void set_zoom(float zoom);
    void zoom_to(float zoom, float duration);
};
