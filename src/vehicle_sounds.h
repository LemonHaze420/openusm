#pragma once

#include <cstdint>

#include "float.hpp"
#include "sound_instance_id.h"

struct vehicle_sounds {
    std::intptr_t m_vtbl;
    bool field_4;
    bool field_5;
    bool field_6;
    float field_8;
    float field_C;
    float field_10;
    float field_14;
    float field_18;
    int field_1C;
    float field_20;
    float field_24;
    float field_28;
    sound_instance_id field_2C;
    bool field_30;
    bool field_31;
    float field_34;
    sound_instance_id field_38;
    sound_instance_id field_3C;
    sound_instance_id field_40;
    sound_instance_id field_44;
    sound_instance_id field_48;
    sound_instance_id field_4C;

    vehicle_sounds();
    void stop_engine_sounds();
    void stop_horn();

    //0x006BB690
    void manage_engine_sounds(Float a2, bool a3);
    void audio_advance(Float dt);
    void manage_tire_sounds(Float dt, bool audible);
    void handle_horn_sounds(Float dt, bool audible);
    void set_horn(bool enabled);
    void play_stopped_sound();
};
