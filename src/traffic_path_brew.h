#pragma once

#include "progress.h"
#include "intersection_manager_brew.h"

struct traffic_path_brew {
    traffic_path_brew();

    progress field_0;
    limited_timer *field_4{};
    progress field_8;
    intersection_manager_brew field_C;
    int vehicle_state{};
    int pedestrian_state{};
    int parking_state{};
    int fixup_state{};
    char *field_40{};
    int field_44{};
};
