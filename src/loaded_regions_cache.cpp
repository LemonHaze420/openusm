#include "loaded_regions_cache.h"

#include "common.h"
#include "fixed_vector.h"
#include "func_wrapper.h"
#include "trace.h"
#include "vector3d.h"
#include "vector4d.h"


namespace loaded_regions_cache {

VALIDATE_SIZE(region, 0x1C);


void get_regions_intersecting_sphere_platform_independent(const vector4d &a1,
                                                          fixed_vector<::region *, 15> *output_array)
{
    CDECL_CALL(0x00565BF0, &a1, output_array);
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
