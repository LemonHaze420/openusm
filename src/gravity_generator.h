#pragma once

#include "force_generator.h"

struct gravity_generator : force_generator {
    bool field_4;
    bool field_5;

    gravity_generator();
    void frame_advance(Float elapsed);
};
