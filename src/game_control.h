#pragma once

#include "device_axis.h"

#include <list.hpp>

enum control_id_t {
    GRAVITY_TOGGLE = 7,
    STOP_PHYSICS = 8,
    SINGLE_STEP = 9,
    SHOW_DEBUG_INFO = 10,
    USERCAM_EQUALS_CHASECAM = 28,
    PLANE_BOUNDS_MOD = 48,
};

enum axis_id_t {};

using device_axis_list_t = typename _std::list<device_axis>;

enum control_t { CT_BOOLEAN, CT_RATIONAL };

struct game_control {
    control_id_t name;
    control_t type;
    device_axis_list_t mapping;

    game_control();
};
