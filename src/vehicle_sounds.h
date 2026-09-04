#pragma once

#include <cstdint>

#include "float.hpp"

struct vehicle_sounds {
    std::intptr_t m_vtbl;
    int field_4[19];

    vehicle_sounds();

    //0x006BB690
    void manage_engine_sounds(Float a2, bool a3);
};
