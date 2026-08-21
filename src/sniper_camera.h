#pragma once

#include "game_camera.h"

struct sniper_camera : game_camera {
    int field_1A0;
    int field_1A4;
    int field_1A8;
    int field_1AC;
    int field_1B0;
    int field_1B4;

    //0x0057D8A0
    sniper_camera(const string_hash &a2, entity *a3);

    void *operator new(size_t size);
};
