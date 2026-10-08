#pragma once

#include "game_camera.h"

struct glam_camera : game_camera {
    float elevation;
    float azimuth;
    float distance;
    float fov;
    string_hash camera_bone;
    string_hash target_bone;
    entity_base_vhandle target;
    glam_camera(const char *a1);
#if STANDALONE_SYSTEM
    static void *native_vtable();
#endif
    void frame_advance(Float dt);
};
