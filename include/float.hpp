#pragma once

struct Float {
    float value{0.0};

    constexpr Float(float p) : value(p) {}

    constexpr operator float() const
    {
        return value;
    }
};
