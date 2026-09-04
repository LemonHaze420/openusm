#include "distance_fader.h"

#include "trace.h"

#include <cassert>

int distance_fader::get_fade_index_for_fade_distance(Float a1)
{
    TRACE("distance_fader::get_fade_index_for_fade_distance");

    int idx = 0;
    float v4 = 3.4028235e38;
    for (auto i = 0u; i < 16u; ++i) {
        auto v2 = std::abs(distance_fader::fade_distances()[i] - a1);
        if (v4 > v2) {
            v4 = v2;
            idx = i;
        }
    }

    return idx;
}

int distance_fader::estimate_fade_index_for_bounding_sphere(Float sphere_radius)
{
    assert(sphere_radius >= 0.0f);
    assert(fade_radii()[0] < 0.0f);

    for (int i = 0; i < 15; ++i) {
        if (sphere_radius >= fade_radii()[i] && fade_radii()[i + 1] >= sphere_radius) {
            return i;
        }
    }

    return 15;
}
