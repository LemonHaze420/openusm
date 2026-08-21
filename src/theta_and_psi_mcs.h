#pragma once

#include "float.hpp"
#include "motion_control_system.h"

#include <cstdint>

struct camera;
struct entity;

struct theta_and_psi_mcs : motion_control_system {
    float m_theta;
    float m_psi;
    float d_theta_for_next_frame;
    float d_psi_for_next_frame;

    theta_and_psi_mcs(entity *a2, Float a3, Float a4);

    void *operator new(size_t size);

    //0x005196E0
    void reset_angles();

    //0x0053B260
    //virtual
    void frame_advance(Float dt);
};
