#pragma once

#include "vector4d.h"

#include <cstdio>

struct matrix4x3 {
    vector4d arr[3];

    matrix4x3() = default;

    auto &operator[](int idx)
    {
        return arr[idx];
    }

    const auto &operator[](int idx) const
    {
        return arr[idx];
    }

    bool operator==(const matrix4x3 &);

    //0x004139A0
    friend vector4d operator*(const vector4d &a2, const matrix4x3 &a3);

    matrix4x3 transposed() const;

    const char *to_string() const;
};
