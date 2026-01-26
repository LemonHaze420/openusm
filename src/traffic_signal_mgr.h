#pragma once

#include "float.hpp"
#include "variable.h"

struct entity;

struct traffic_signal_mgr {
    traffic_signal_mgr();

    static void frame_advance(Float a1);

    //0x005528D0
    static void switch_to_next_state();

    //0x0054E140
    static void add_traffic_light(entity *a1, bool a2);

    struct state_timer_t {
        float field_0;
    };

    static inline Var<state_timer_t> m_state_timer{0x0095CAA0};
};
