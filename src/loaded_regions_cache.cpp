#include "loaded_regions_cache.h"

#include "common.h"
#include "fixed_vector.h"
#include "func_wrapper.h"
#include "region.h"

#include "trace.h"
#include "vector3d.h"
#include "vector4d.h"

namespace loaded_regions_cache {

VALIDATE_SIZE(region, 0x1C);

void get_regions_intersecting_sphere_platform_independent(
    const vector4d &sphere, fixed_vector<::region *, 15> *output_array)
{
    assert(output_array != nullptr);
    const vector3d sphere_min{
        sphere[0] - sphere[3], sphere[1] - sphere[3], sphere[2] - sphere[3]};
    const vector3d sphere_max{
        sphere[0] + sphere[3], sphere[1] + sphere[3], sphere[2] + sphere[3]};
    for (const auto &cached : regions()) {
        if (cached.region_allocation_index == UINT16_MAX) {
            continue;
        }
        const bool overlaps =
            cached.field_C[0] >= sphere_min[0] && cached.field_0[0] <= sphere_max[0] &&
            cached.field_C[1] >= sphere_min[1] && cached.field_0[1] <= sphere_max[1] &&
            cached.field_C[2] >= sphere_min[2] && cached.field_0[2] <= sphere_max[2];
        if (overlaps && ::region::all_regions != nullptr &&
            output_array->size() < 15) {
            output_array->push_back(
                &::region::all_regions[cached.region_allocation_index]);
        }
    }
}

void get_regions_intersecting_sphere(const vector3d &a1, Float a2, fixed_vector<::region *, 15> *a3)
{
    TRACE("loaded_regions_cache::get_regions_intersecting_sphere");

    vector4d v5;
    v5[0] = a1[0];
    v5[1] = a1[1];
    v5[2] = a1[2];
    v5[3] = a2;
    get_regions_intersecting_sphere_platform_independent(v5, a3);
}

}  // namespace loaded_regions_cache
