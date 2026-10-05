#include "limbo_area.h"
#include "variable.h"
#include "vector3d.h"
#include "vector4d.h"

#include "utility.h"

bool limbo_area::sphere_intersects_unsafe_area(const vector3d &center, Float radius)
{
    static auto &bounds = var<vector4d *>(0x00921E44);
    static auto &block_count = var<int>(0x0095C2F4);
    if (bounds == nullptr || block_count <= 0) {
        return false;
    }
    const float min_x = center[0] - radius.value;
    const float max_x = center[0] + radius.value;
    const float min_z = center[2] - radius.value;
    const float max_z = center[2] + radius.value;
    for (int block = 0; block < block_count; ++block) {
        const auto *soa = bounds + block * 4;
        for (int lane = 0; lane < 4; ++lane) {
            if (soa[0][lane] >= min_x && soa[1][lane] >= min_z && soa[2][lane] <= max_x && soa[3][lane] <= max_z) {
                return true;
            }
        }
    }
    return false;
}
