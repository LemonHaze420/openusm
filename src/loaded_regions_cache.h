#pragma once

#include <cstdint>

#include "float.hpp"
#include "variable.h"
#include "vector3d.h"

struct region;
struct vector4d;

template <typename T, uint32_t>
struct fixed_vector;

namespace loaded_regions_cache {

struct region {
    vector3d field_0;
    vector3d field_C;
    uint16_t region_allocation_index;
    uint16_t field_1A;
};

extern void get_regions_intersecting_sphere_platform_independent(const vector4d &, fixed_vector<::region *, 15> *);

extern void get_regions_intersecting_sphere(const vector3d &a1, Float a2, fixed_vector<::region *, 15> *a3);

inline Var<region[9]> regions{0x009222F8};
}  // namespace loaded_regions_cache
