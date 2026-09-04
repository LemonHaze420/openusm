#pragma once

#include "color.h"

#include <cstdint>

struct color32 {
    uint8_t field_0[4]{};

    color32() = default;

    color32(uint32_t value)
    {
        set_red((value >> 0u) & 0x000000FF);
        set_green((value >> 8u) & 0x000000FF);
        set_blue((value >> 16u) & 0x000000FF);
        set_alpha(value >> 24u);
    }

    color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : field_0{r, g, b, a} {}

    auto &operator[](int i)
    {
        return field_0[i];
    }

    auto &operator[](int i) const
    {
        return field_0[i];
    }

    bool operator!=(const color32 &a2) const
    {
        return this->field_0[0] != a2.field_0[0] && this->field_0[1] != a2.field_0[1] &&
               this->field_0[2] != a2.field_0[2] && this->field_0[3] != a2.field_0[3];
    }

    color to_color();

    uint8_t get_alpha();

    void set_alpha(uint8_t v);

    void set_blue(uint8_t v);

    void set_green(uint8_t v);

    void set_red(uint8_t v);

    static int to_int(color32 color);
};
