#include "ai_quad_path.h"

#include "ai_quad_path_cell.h"
#include "common.h"
#include "region.h"
#include "ai_quad_path_exit.h"
#include "collide.h"
#include <cfloat>

#include <func_wrapper.h>

#include <cassert>

VALIDATE_SIZE(ai_quad_path, 0x34);

ai_quad_path::ai_quad_path() {}

void ai_quad_path::un_mash(void *buffer_ptr, region *reg, int a4, int a5, int a6)
{
    if constexpr (1) {
        this->field_18 = reg;
        if (reg->get_district_id() != this->field_1C) {
            auto &scene_id = reg->get_scene_id(false);

            auto *str = scene_id.c_str();
            auto district_id = this->field_18->get_district_id();

            sp_log("Path graph %02d/%02d erroneously thinks it is in district %d when it is in "
                   "district %d.\n"
                   "  This is an art bug in the quad paths in region %s.",
                   this->field_1C,
                   this->field_1E,
                   this->field_1C,
                   district_id,
                   str);

            assert(0);
        }

        this->field_24 = (ai_quad_path_cell *)((char *)this->field_24 + (uintptr_t)buffer_ptr + a4);
        this->field_20 += (int)buffer_ptr + a5;

        for (int i = 0; i < this->field_2A; ++i) {
            this->field_24[i].un_mash(buffer_ptr, a4, a6);
        }

        this->field_2C = 0;
        this->field_30 = 0;

    } else {
        THISCALL(0x00464FA0, this, buffer_ptr, reg, a4, a5, a6);
    }
}

bool ai_quad_path::find_exit_to_district(int district, int path, const vector3d &position, vector3d &exit_position,
                                         ai_quad_path_cell *&exit_cell) const
{
    float distance = FLT_MAX;
    const auto *exits = reinterpret_cast<const ai_quad_path_exit *>(field_20);
    for (unsigned int i = 0; i < field_28; ++i) {
        const auto &exit = exits[i];
        if (exit.district != district || (path != -1 && exit.path != path))
            continue;
        auto &cell = field_24[exit.cell];
        vector3d point;
        closest_point_segment(position, cell.field_0[exit.edge], cell.field_0[(exit.edge + 1) % 4], point);
        const float candidate = (point - position).length2();
        if (candidate < distance) {
            distance = candidate;
            exit_position = point;
            exit_cell = &cell;
        }
    }
    return distance < FLT_MAX;
}

ai_quad_path_cell *ai_quad_path::find_exit_cell_to_path(const ai_quad_path &path, const vector3d &position,
                                                        vector3d &exit_position) const
{
    ai_quad_path_cell *cell = nullptr;
    return find_exit_to_district(path.field_1C, path.field_1E, position, exit_position, cell) ? cell : nullptr;
}

bool ai_quad_path::check_points_in_cells(const vector3d &position, float tolerance, ai_quad_path_cell **inside,
                                         ai_quad_path **nearest_path, ai_quad_path_cell **nearest_cell,
                                         float *nearest_distance)
{
    const auto *bounds = reinterpret_cast<const float *>(field_0);
    if (nearest_path == nullptr &&
        (position.x <= bounds[0] || position.x >= bounds[3] || position.z <= bounds[2] || position.z >= bounds[5]))
        return false;
    for (unsigned int i = 0; i < field_2A; ++i) {
        if (field_24[i].is_point_in_cell(position, tolerance)) {
            if (inside != nullptr)
                *inside = &field_24[i];
            return true;
        }
    }
    if (nearest_path != nullptr) {
        for (unsigned int i = 0; i < field_2A; ++i) {
            const float distance = field_24[i].is_point_near_cell(position);
            if (distance < *nearest_distance) {
                *nearest_distance = distance;
                *nearest_cell = &field_24[i];
                *nearest_path = this;
            }
        }
    }
    return false;
}
