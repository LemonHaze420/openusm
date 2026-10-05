#pragma once

#include "ai_tentacle_engine.h"

#include <cstdint>
#include "float.hpp"
#include "vector3d.h"

struct ai_tentacle_info;
struct dangler;

struct ai_tentacle_dangle : ai_tentacle_engine {
    dangler *tentacle_dangler;
    bool field_20;
    bool field_21;

    //0x00470B70
    ai_tentacle_dangle(ai_tentacle_info *a2);
    ~ai_tentacle_dangle();
    static void *native_vtable();
    void setup(float length, const vector3d &velocity, bool update_end, bool collision);
    bool frame_advance(Float dt, bool modifier);
    void set_modifier(ai_tentacle_engine *modifier);
    void set_length(float length);
    float get_length() const;
};
