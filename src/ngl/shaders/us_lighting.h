#pragma once

#include "variable.h"

struct entity;
struct light_source;

extern void us_lighting_switch_time_of_day(int a1);

inline Var<entity *[4]> usl_skydomes{0x0095630C};

inline Var<float[1][4]> usl_street_tod_color{0x009566E0};

inline Var<light_source *[4]> usl_suns{0x0095631C};
