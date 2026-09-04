#pragma once

#include "ai_tentacle_engine.h"

#include <cstdint>

struct ai_tentacle_info;
struct dangler;

struct ai_tentacle_dangle : ai_tentacle_engine {
    dangler *tentacle_dangler;
    bool field_20;
    bool field_21;

    //0x00470B70
    ai_tentacle_dangle(ai_tentacle_info *a2);
};
