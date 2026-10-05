#pragma once

#include "float.hpp"
#include "variable.h"

struct distance_fader {
    //0x0051A200
    static int get_fade_index_for_fade_distance(Float a1);

    //0x0051A330
    static int estimate_fade_index_for_bounding_sphere(Float sphere_radius);

#if STANDALONE_SYSTEM
    static const auto &fade_radii()
    {
        static constexpr float values[16]{
            -1.0f, 0.0f, 0.2f, 0.5f, 1.0f, 1.5f, 2.0f, 2.5f,
            3.0f, 3.5f, 4.0f, 7.0f, 10.0f, 15.0f, 20.0f, 40.0f};
        return values;
    }

    static const auto &fade_distances()
    {
        static constexpr float values[16]{
            10.0f, 15.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f,
            80.0f, 90.0f, 100.0f, 150.0f, 200.0f, 280.0f, 500.0f, 100000.0f};
        return values;
    }

    static const auto &fade_distances2()
    {
        static constexpr float values[16]{
            100.0f, 225.0f, 400.0f, 900.0f, 1600.0f, 2500.0f, 3600.0f, 4900.0f,
            6400.0f, 8100.0f, 10000.0f, 22500.0f, 40000.0f, 78400.0f, 250000.0f, 10000000000.0f};
        return values;
    }
#else
    static inline Var<float[16]> fade_radii{0x00921EC8};

    static inline Var<float[16]> fade_distances{0x00921E48};

    static inline Var<float[16]> fade_distances2{0x00921E88};
#endif
};

inline constexpr auto FADE_DISTANCE_MASK = 0xF;
