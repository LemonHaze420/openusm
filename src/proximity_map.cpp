#include "proximity_map.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "sector2d.h"
#include "vector3d.h"

#include <cassert>

VALIDATE_SIZE(proximity_map_construction_leaf, 0x1C);

int proximity_map::traverse_point(const vector3d &a2, subdivision_visitor &a3)
{
    return THISCALL(0x005115E0, this, &a2, &a3);
}

void proximity_map::map_vector3d_to_cell_index(const vector3d &a2, cell_index *a3)
{
    THISCALL(0x0055E900, this, &a2, a3);
}

void proximity_map::map_vector3d_range_to_cell_range_with_swapping(const vector3d &a2, const vector3d &a3,
                                                                   cell_index &start_index, cell_index &end_index)
{
    this->map_vector3d_to_cell_index(a2, &start_index);
    this->map_vector3d_to_cell_index(a3, &end_index);
    if (start_index.x > end_index.x) {
        std::swap(start_index.x, end_index.x);
    }

    if (start_index.y > end_index.y) {
        std::swap(start_index.y, end_index.y);
    }
}

int proximity_map::traverse_sphere(const vector3d &a2, Float a3, subdivision_visitor *a4)
{
    TRACE("proximity_map::traverse_sphere");

    assert(initialized);

    if constexpr (0) {
        auto v4 = a3 + LARGE_EPSILON;

        vector3d v24 = a2;
        v24 = v24 - v4;

        vector3d a2a = a2;
        a2a = a2a + v4;

        cell_index start_index;
        cell_index end_index;
        this->map_vector3d_range_to_cell_range_with_swapping(v24, a2a, start_index, end_index);

        int x = start_index.x;
    LABEL_9:
        if (x > end_index.x) {
            return 0;
        }

        int status;
        for (int y = start_index.y;; ++y) {
            if (y > end_index.y) {
                ++x;
                goto LABEL_9;
            }

            //auto cell_entries = this->get_cell_entries(cell_index {x, y});
            //auto status = this->traverse_sphere(cell_entries, a2, a3, a4);
            if (status != 0) {
                break;
            }
        }

        assert(status == STOP_VISITING_WITHIN_THIS_CELL || status == STOP_VISITING_FARTHER_CELLS);

        return status;

    } else {
        return THISCALL(0x005229B0, this, &a2, a3, a4);
    }
}

void proximity_map::traverse_sector_raster(const sector2d &sec, Float sector_radius, subdivision_visitor &visitor)
{
    assert(sector_radius > EPSILON);
    assert(is_initialized());

    THISCALL(0x00522680, this, &sec, sector_radius, &visitor);
}

bool proximity_map::is_initialized()
{
    return this->initialized;
}

void dynamic_proximity_map::init(dynamic_proximity_map_stack *alloc, fixed_pool *a3, int number_of_cells_arg,
                                 const vector3d &a5, const vector3d &a6, subdivision_node::type_t cell_node_type)
{
    assert(cell_node_type == subdivision_node::DYNAMIC_ENTITY_LIST_NODE ||
           cell_node_type == subdivision_node::DYNAMIC_LEAF_LIST_NODE);
    assert(alloc != nullptr);
    assert(number_of_cells == number_of_cells_arg);

    assert(map_type == DYNAMIC);

    THISCALL(0x00522500, this, alloc, a3, number_of_cells_arg, &a5, &a6, cell_node_type);
}

proximity_map_construction_leaf::proximity_map_construction_leaf(region *a2, const vector3d &a1, const vector3d &a4)
{
    this->field_0.r = a2;
    this->field_4 = a1;
    this->field_10 = a4;
}
