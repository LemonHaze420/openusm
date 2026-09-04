#pragma once

#include <cstdint>

struct color {
    float r, g, b, a;

    void *operator new(size_t);

    color operator*(float f) const
    {
        return color{this->r * f, this->g * f, this->b * f, this->a * f};
    }

    color operator+(const color &c) const
    {
        return color{this->r + c.r, this->g + c.g, this->b + c.b, this->a + c.a};
    }
};
