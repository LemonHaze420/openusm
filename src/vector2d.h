#pragma once

#include "float.hpp"

struct vector2d {
    float x, y;

    vector2d() = default;

    vector2d(float a1, float a2) : x(a1), y(a2) {}

    const float &operator[](int idx) const {
        return (&x)[idx];
    }

    float &operator[](int idx) {
        return (&x)[idx];
    }

    vector2d operator+(const vector2d &a1) const;

    vector2d operator-(const vector2d &a3) const;

    vector2d operator*(float a1) const;

    vector2d & operator*=(float a2);

    vector2d & operator/=(float a2);

    const char *to_string() const;

    float length2() const;

    float length() const;
};
