#pragma once

#include "fixed_vector.h"
#include "float.hpp"
#include "plane.h"

#include <cstdint>

struct hull {
    fixed_vector<plane, 7> field_0;

    hull() = default;

    //0x005C7400
    void add_face(const plane &a2);

    bool sub_5CC030(Float a2, Float a3, Float a4, Float a5);
};
