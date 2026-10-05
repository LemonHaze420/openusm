#include "loaded_regions_cache.h"

#include "common.h"
#include "fixed_vector.h"
#include "func_wrapper.h"
#include "region.h"
#include "subdivision_obb.h"

#include "trace.h"
#include "vector3d.h"
#include "vector4d.h"

namespace loaded_regions_cache {

VALIDATE_SIZE(region, 0x1C);

#if STANDALONE_SYSTEM
namespace {

void calculate_region_boundaries()
{
    static auto &bounds = []() -> vector4d *& {
        auto &pointer = var<vector4d *>(0x00921E44);
        pointer = var<vector4d[32]>(0x0095F978);
        return pointer;
    }();
    auto &block_count = var<int>(0x0095C2F4);

    ::region *boundary_regions[32];
    unsigned int count = 0;
    block_count = 0;
    for (const auto &cached : regions()) {
        if (cached.region_allocation_index == UINT16_MAX) {
            continue;
        }
        auto *loaded_region = &::region::all_regions[cached.region_allocation_index];
        if ((loaded_region->flags & (0x100u | 0x40000u)) != 0) {
            continue;
        }
        for (int i = 0; i < loaded_region->get_num_neighbors(); ++i) {
            auto *neighbor = loaded_region->get_neighbor(i);
            if ((neighbor->flags & (0x10u | 0x100u | 0x40000u)) != 0) {
                continue;
            }
            unsigned int j = 0;
            while (j < count && boundary_regions[j] != neighbor) {
                ++j;
            }
            if (j != count) {
                continue;
            }
            boundary_regions[count] = neighbor;
            vector3d min_extent;
            vector3d max_extent;
            neighbor->obb->get_extents(&min_extent, &max_extent);
            auto *block = bounds + (count / 4) * 4;
            const auto lane = count % 4;
            block[0][lane] = max_extent.x + 20.0f;
            block[1][lane] = max_extent.z + 20.0f;
            block[2][lane] = min_extent.x - 20.0f;
            block[3][lane] = min_extent.z - 20.0f;
            ++count;
        }
    }
    while ((count & 3u) != 0) {
        auto *block = bounds + (count / 4) * 4;
        const auto lane = count % 4;
        block[0][lane] = -std::numeric_limits<float>::max();
        block[1][lane] = -std::numeric_limits<float>::max();
        block[2][lane] = std::numeric_limits<float>::max();
        block[3][lane] = std::numeric_limits<float>::max();
        ++count;
    }
    block_count = count / 4;
}

}  // namespace
#endif

void add(::region *loaded_region)
{
#if STANDALONE_SYSTEM
    const auto index = loaded_region - ::region::all_regions;
    for (auto &cached : regions()) {
        if (cached.region_allocation_index == UINT16_MAX) {
            loaded_region->obb->get_extents(&cached.field_0, &cached.field_C);
            cached.region_allocation_index = static_cast<uint16_t>(index);
            calculate_region_boundaries();
            return;
        }
        if (cached.region_allocation_index == index) {
            return;
        }
    }
#else
    CDECL_CALL(0x005401C0, loaded_region);
#endif
}

void remove(::region *loaded_region)
{
#if STANDALONE_SYSTEM
    const auto index = loaded_region - ::region::all_regions;
    for (auto &cached : regions()) {
        if (cached.region_allocation_index == index) {
            cached.field_0 = vector3d{std::numeric_limits<float>::max()};
            cached.field_C = vector3d{-std::numeric_limits<float>::max()};
            cached.region_allocation_index = UINT16_MAX;
            return;
        }
    }
#else
    CDECL_CALL(0x00519A70, loaded_region);
#endif
}

void get_regions_intersecting_sphere_platform_independent(const vector4d &sphere,
                                                          fixed_vector<::region *, 15> *output_array)
{
    assert(output_array != nullptr);
    const vector3d sphere_min{sphere[0] - sphere[3], sphere[1] - sphere[3], sphere[2] - sphere[3]};
    const vector3d sphere_max{sphere[0] + sphere[3], sphere[1] + sphere[3], sphere[2] + sphere[3]};
    for (const auto &cached : regions()) {
        if (cached.region_allocation_index == UINT16_MAX) {
            continue;
        }
        const bool overlaps = cached.field_C[0] >= sphere_min[0] && cached.field_0[0] <= sphere_max[0] &&
                              cached.field_C[1] >= sphere_min[1] && cached.field_0[1] <= sphere_max[1] &&
                              cached.field_C[2] >= sphere_min[2] && cached.field_0[2] <= sphere_max[2];
        if (overlaps && ::region::all_regions != nullptr && output_array->size() < 15) {
            output_array->push_back(&::region::all_regions[cached.region_allocation_index]);
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

void get_regions_intersecting_box(const vector3d &start, const vector3d &end, fixed_vector<::region *, 15> *output,
                                  const vector3d &margin)
{
    vector3d minimum;
    for (int axis = 0; axis != 3; ++axis)
        minimum[axis] = (start[axis] < end[axis] ? start[axis] : end[axis]) - margin[axis];
    const vector3d sum = start + end;
    for (const auto &cached : regions()) {
        if (cached.region_allocation_index == UINT16_MAX)
            continue;
        bool overlaps = true;
        for (int axis = 0; axis != 3; ++axis) {
            const float reflected_max = sum[axis] - cached.field_0[axis];
            const float overlap = reflected_max < cached.field_C[axis] ? reflected_max : cached.field_C[axis];
            if (!(overlap >= minimum[axis])) {
                overlaps = false;
                break;
            }
        }
        if (overlaps)
            output->push_back(&::region::all_regions[cached.region_allocation_index]);
    }
}

}  // namespace loaded_regions_cache
