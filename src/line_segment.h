#pragma once

#include "vector3d.h"

struct line_segment_t {
    vector3d field_0;
    vector3d field_C;
    vector3d field_18;
    vector3d field_24;
    int field_30;
    bool field_34;

    line_segment_t();

    line_segment_t(const vector3d &a2, const vector3d &a3);
};
