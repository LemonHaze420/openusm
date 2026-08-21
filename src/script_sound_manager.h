#pragma once

#include "float.hpp"
#include "sound_instance_id.h"
#include "variable.h"

namespace script_sound_manager {

//0x0065F0A0
extern void create_inst();

extern void delete_inst();

//0x0065F240
extern void frame_advance(Float a1);
}  // namespace script_sound_manager

struct script_sound_instance_slot {
    sound_instance_id field_0;
    int field_4;
    float field_8[8];
    int field_28;
    string_hash field_2C;
    int field_30;
    int field_34;
    bool field_38;
    bool field_39;
    int field_3C;
};

extern Var<script_sound_instance_slot *> s_script_sound_instance_slots;
