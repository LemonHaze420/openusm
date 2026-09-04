#include "ai_region_paths.h"

#include "ai_quad_path.h"
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"

#include <cassert>

VALIDATE_SIZE(ai_region_paths, 0x40);

ai_region_paths::ai_region_paths() {}

ai_quad_path *ai_region_paths::get_quad_path_for_point(const vector3d &a2, Float a3, ai_quad_path_cell **a4, bool a5,
                                                       ai_quad_path *a6)
{
    return (ai_quad_path *)THISCALL(0x0046EFF0, this, &a2, a3, a4, a5, a6);
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
