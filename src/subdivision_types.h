#pragma once

#include <cstdint>

using subdivision_uint16 = uint16_t;
using subdivision_int16 = int16_t;

struct subdivision_uint16_vector3d {
    enum { MAX = 65535 };

    void set(subdivision_uint16 x_arg, subdivision_uint16 y_arg, subdivision_uint16 z_arg)
    {
        x = x_arg;
        y = y_arg;
        z = z_arg;
    }

    subdivision_uint16 x, y, z;
};

struct subdivision_int16_vector3d {
    enum { MAX = 32767 };

    void set(subdivision_int16 x_arg, subdivision_int16 y_arg, subdivision_int16 z_arg)
    {
        x = x_arg;
        y = y_arg;
        z = z_arg;
    }

    subdivision_int16 x, y, z;
};
