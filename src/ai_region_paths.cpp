#include "ai_region_paths.h"

#include "ai_quad_path.h"
#include "ai_quad_path_cell.h"
#include <cfloat>
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(ai_region_paths, 0x40);

ai_region_paths::ai_region_paths() {}

ai_quad_path *ai_region_paths::get_quad_path_for_point(const vector3d &a2, Float a3, ai_quad_path_cell **a4, bool a5,
                                                       ai_quad_path *a6)
{
    ai_quad_path *nearest_path = nullptr;
    ai_quad_path_cell *nearest_cell = nullptr;
    float nearest_distance = FLT_MAX;
    auto check = [&](ai_quad_path *path) {
        const auto *bounds = reinterpret_cast<const float *>(path->field_0);
        if (!a5 && (a2.x <= bounds[0] || a2.x >= bounds[3]
                || a2.z <= bounds[2] || a2.z >= bounds[5]))
            return false;
        for (int index = 0; index < path->field_2A; ++index) {
            auto *cell = &path->field_24[index];
            if (cell->is_point_in_cell(a2, a3.value)) {
                if (a4)
                    *a4 = cell;
                return true;
            }
        }
        if (a5) {
            for (int index = 0; index < path->field_2A; ++index) {
                auto *cell = &path->field_24[index];
                const float distance = cell->is_point_near_cell(a2);
                if (distance < nearest_distance) {
                    nearest_distance = distance;
                    nearest_path = path;
                    nearest_cell = cell;
                }
            }
        }
        return false;
    };
    if (a6) {
        if (check(a6))
            return a6;
    } else {
        for (int index = 0; index < quad_path_table_count; ++index) {
            auto *path = &get_quad_path_internal()[index];
            if (check(path))
                return path;
        }
    }
    if (a5 && a4)
        *a4 = nearest_cell;
    return a5 ? nearest_path : nullptr;
}

ai_quad_path *ai_region_paths::get_quad_path_internal()
{
    assert(quad_path_table == (ai_quad_path *)(((uintptr_t)this) + sizeof(ai_region_paths)));
    this->quad_path_table = (ai_quad_path *)&this[1];
    return this->quad_path_table;
}

ai_quad_path *ai_region_paths::get_quad_path(int index)
{
    assert(index >= 0);
    assert(index < quad_path_table_count);
    return &(this->get_quad_path_internal()[index]);
}

void ai_region_paths::un_mash(void *buffer_ptr, int *a3, region *reg)
{
    TRACE("ai_region_paths::un_mash");

    if constexpr (1) {
        assert(strcmp(id, "RGNPTHS") == 0);

        auto v5 = this->quad_path_table_count;
        auto v6 = 5 * this->field_30;
        this->field_8 = reg;
        auto v7 = 52 * v5 + 64;
        auto v8 = v7 + 16 * v6;
        auto v9 = v8 + 12 * this->field_34;
        auto v13 = v9 + 4 * this->field_38;

        assert(quad_path_table == nullptr &&
               "Don't skip this assert it's a crash we're trying to track down.  Get somebody.");

        this->quad_path_table = (ai_quad_path *)((char *)buffer_ptr + sizeof(ai_region_paths));

        assert(quad_path_table == (ai_quad_path *)(((uintptr_t)this) + sizeof(ai_region_paths)));

        this->field_3C = static_cast<char *>(buffer_ptr) + v8;
        for (int i = 0; i < this->quad_path_table_count; ++i) {
            auto *path_ptr = this->get_quad_path(i);
            path_ptr->un_mash(buffer_ptr, reg, v7, v8, v9);
        }

        *a3 = v13;

    } else {
        THISCALL(0x0046F0F0, this, buffer_ptr, a3, reg);
    }
}
