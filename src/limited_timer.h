#pragma once

#include "float.hpp"

#include <cstdint>

struct limited_timer_base {
    uint32_t field_0;  //ms

    limited_timer_base();

    void reset();

    double elapsed() const;
};

struct limited_timer : limited_timer_base {
    float field_4;

    limited_timer() = default;

    limited_timer(Float a1);

    ~limited_timer();
};
