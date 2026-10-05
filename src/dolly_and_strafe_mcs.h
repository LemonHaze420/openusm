#pragma once

#include "float.hpp"
#include "motion_control_system.h"

#include <cstdint>

struct entity;

struct dolly_and_strafe_mcs : motion_control_system {
    float m_dolly;
    float m_strafe;
    float m_lift;

    dolly_and_strafe_mcs(entity *a2);

    //0x00526940
    void do_dolly(Float a2);

    //0x00526A30
    void do_strafe(Float a2);

    //0x00526B20
    void do_lift(Float a2);

    //0x0052E6B0
    //virtual
    void frame_advance(Float a2);
};
