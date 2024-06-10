#pragma once

#include "entity_base_interface.h"

#include "float.hpp"
#include "vector3d.h"

#include <cstdint>

struct shared_sound_interface_info;

struct sound_emitter_id
{
    int field_0;
};

struct sound_interface : entity_base_interface {
    int field_4;
    bool dynamic;
    sound_emitter_id field_C;
    shared_sound_interface_info *field_10;
    vector3d field_14;
    float field_20;
    float field_24;

    sound_interface();

    //0x004D1910
    static void frame_advance_all_sound_ifc(Float a3);
};


extern void sound_interface_patch();
