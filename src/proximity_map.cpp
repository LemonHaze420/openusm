#include "proximity_map.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "sector2d.h"
#include "vector3d.h"
#include "fixed_vector.h"
#include "lego_render_visitor.h"
#include "subdivision_static_region_list.h"
#include "subdivision_visitor.h"
#include "vector2d.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include <cassert>

VALIDATE_SIZE(proximity_map_construction_leaf, 0x1C);

int proximity_map::traverse_point(const vector3d &a2, subdivision_visitor &a3)
{
#if STANDALONE_SYSTEM
    assert(initialized);
    cell_index index;
    map_vector3d_to_cell_index(a2, &index);
    const auto *directory = reinterpret_cast<const uint16_t *>(reinterpret_cast<const uint32_t *>(this) + field_8);
    const int16_t offset = static_cast<int16_t>(directory[index.x + (index.y << log_number_of_cells)]);
    if (offset == 0)
        return 0;
    const auto &node = *reinterpret_cast<const subdivision_node *>(reinterpret_cast<const uint32_t *>(this) + offset);
    const traverse_test test{1, a2, 0.0f};
    return visit_cell_node(node, a3, &test);
#else
    return THISCALL(0x005115E0, this, &a2, &a3);
#endif
}

void proximity_map::map_vector3d_to_cell_index(const vector3d &a2, cell_index *a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto x = a2.x - this->field_C.x;
        auto z = a2.z - this->field_C.z;

        if (x < 0.0f) {
            x = EPSILON;
        } else if (x >= this->field_48.x) {
            x = this->field_48.x;
        }

        if (z < 0.0f) {
            z = EPSILON;
        } else if (z >= this->field_48.z) {
            z = this->field_48.z;
        }

        const auto number_of_cells = static_cast<float>(this->number_of_cells);
        a3->x = static_cast<int16_t>(number_of_cells * this->field_3C.x * x);
        a3->y = static_cast<int16_t>(number_of_cells * this->field_3C.z * z);
    } else {
        THISCALL(0x0055E900, this, &a2, a3);
    }
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

int proximity_map::traverse_sphere(const vector3d &position, Float radius, subdivision_visitor *visitor)
{
#if STANDALONE_SYSTEM
    assert(initialized);
    const float extent = radius + LARGE_EPSILON;
    cell_index first, last;
    map_vector3d_range_to_cell_range_with_swapping(position - extent, position + extent, first, last);
    const auto *directory = reinterpret_cast<const uint16_t *>(reinterpret_cast<const uint32_t *>(this) + field_8);
    const traverse_test test{0, position, radius};
    for (int x = first.x; x <= last.x; ++x) {
        for (int y = first.y; y <= last.y; ++y) {
            const int16_t offset = static_cast<int16_t>(directory[x + (y << log_number_of_cells)]);
            if (offset == 0)
                continue;
            const auto &node =
                *reinterpret_cast<const subdivision_node *>(reinterpret_cast<const uint32_t *>(this) + offset);
            const int status = visit_cell_node(node, *visitor, &test);
            if (status != 0)
                return status;
        }
    }
    return 0;
#else
    return THISCALL(0x005229B0, this, &position, radius, visitor);
#endif
}

void proximity_map::traverse_sector_raster(const sector2d &sec, Float sector_radius, subdivision_visitor &visitor)
{
    assert(sector_radius > EPSILON);
    assert(is_initialized());

#if STANDALONE_SYSTEM
    uint32_t rows[32]{};
    const vector2d origin{sec.field_10, sec.field_14};
    const vector2d left{sec.field_0, sec.field_4};
    const vector2d right{sec.field_8, sec.field_C};

    const float tangent = std::tan(sec.fov * 0.25f);
    const float expanded_radius = sector_radius / std::sqrt(1.0f - tangent * tangent);
    const vector2d arc_left = origin + left * expanded_radius;
    const vector2d arc_right = origin + right * expanded_radius;
    vector2d middle = (left + right) * (expanded_radius * 0.5f);
    const float middle_length_squared = middle.x * middle.x + middle.y * middle.y;
    if (middle_length_squared > 9.99999944e-11f)
        middle *= 1.0f / std::sqrt(middle_length_squared);
    middle = origin + middle * expanded_radius;
    mark_raster_line(arc_left, middle, rows);
    mark_raster_line(middle, arc_right, rows);
    mark_raster_line(origin, origin + left * sector_radius, rows);
    mark_raster_line(origin, origin + right * sector_radius, rows);
    visit_raster_rows(rows, visitor);
#else
    THISCALL(0x00522680, this, &sec, sector_radius, &visitor);
#endif
}


void proximity_map::traverse_convex_hull_raster(const fixed_vector<vector2d, 14> &points, subdivision_visitor &visitor)
{
#if STANDALONE_SYSTEM
    assert(initialized);
    uint32_t rows[32]{};
    assert(number_of_cells <= 32);
    for (uint32_t edge = 0; edge < points.m_size; ++edge) {
        const auto &from = points.m_data[edge];
        const auto &to = points.m_data[(edge + 1) % points.m_size];
        mark_raster_line(from, to, rows);
    }
    visit_raster_rows(rows, visitor);
#else
    THISCALL(0x005225F0, this, &points, &visitor);
#endif
}

void proximity_map::mark_raster_line(const vector2d &from, const vector2d &to, uint32_t (&rows)[32])
{
    cell_index cell, end;
    map_vector3d_to_cell_index(vector3d{from.x, 0.0f, from.y}, &cell);
    map_vector3d_to_cell_index(vector3d{to.x, 0.0f, to.y}, &end);
    const float dx = to.x - from.x;
    const float dz = to.y - from.y;
    if (std::abs(dx) <= LARGE_EPSILON && std::abs(dz) <= LARGE_EPSILON) {
        rows[cell.y] |= uint32_t{1} << cell.x;
        return;
    }
    const int step_x = (dx > 0.0f) - (dx < 0.0f);
    const int step_z = (dz > 0.0f) - (dz < 0.0f);
    bool finished_x = std::abs(dx) <= EPSILON;
    bool finished_z = std::abs(dz) <= EPSILON;
    const float delta_x = finished_x ? 0.0f : std::abs(field_30.x / dx);
    const float delta_z = finished_z ? 0.0f : std::abs(field_30.z / dz);
    float next_x = finished_x ? std::numeric_limits<float>::max()
                              : ((cell.x + (step_x > 0)) * field_30.x + field_C.x - from.x) / dx;
    float next_z = finished_z ? std::numeric_limits<float>::max()
                              : ((cell.y + (step_z > 0)) * field_30.z + field_C.z - from.y) / dz;
    for (int iteration = 0;; ++iteration) {
        rows[cell.y] |= uint32_t{1} << cell.x;
        if ((next_x < next_z || finished_z) && !finished_x) {
            cell.x += step_x;
            next_x += delta_x;
            if (cell.x < 0 || cell.x >= number_of_cells) {
                cell.x = static_cast<int16_t>(std::clamp<int>(cell.x, 0, number_of_cells - 1));
                finished_x = true;
            }
        } else {
            cell.y += step_z;
            next_z += delta_z;
            if (cell.y < 0 || cell.y >= number_of_cells) {
                cell.y = static_cast<int16_t>(std::clamp<int>(cell.y, 0, number_of_cells - 1));
                finished_z = true;
            }
        }
        const bool reached_x = step_x > 0 ? cell.x >= end.x : cell.x <= end.x;
        const bool reached_z = step_z > 0 ? cell.y >= end.y : cell.y <= end.y;
        if ((reached_x && reached_z) || (finished_x && finished_z)) {
            rows[cell.y] |= uint32_t{1} << cell.x;
            break;
        }
        if (iteration >= 200)
            break;
    }
}

void proximity_map::visit_raster_rows(const uint32_t (&rows)[32], subdivision_visitor &visitor)
{
    const auto *directory = reinterpret_cast<const uint16_t *>(reinterpret_cast<const uint32_t *>(this) + field_8);
    for (int y = 0; y < 32; ++y) {
        uint32_t mask = rows[y];
        if (mask == 0)
            continue;
        const uint32_t first = mask & ~(mask - 1);
        mask |= mask >> 1;
        mask |= mask >> 2;
        mask |= mask >> 4;
        mask |= mask >> 8;
        mask |= mask >> 16;
        mask &= ~(first - 1);
        for (int x = 0; x < number_of_cells; ++x) {
            if ((mask & (uint32_t{1} << x)) == 0)
                continue;
            const int16_t offset = static_cast<int16_t>(directory[x + (y << log_number_of_cells)]);
            if (offset == 0)
                continue;
            const auto &node =
                *reinterpret_cast<const subdivision_node *>(reinterpret_cast<const uint32_t *>(this) + offset);
            visit_cell_node(node, visitor, nullptr);
        }
    }
}


int proximity_map::visit_cell_node(const subdivision_node &node, subdivision_visitor &visitor,
                                   const traverse_test *test)
{
    if (node.get_type() == subdivision_node::DYNAMIC_LEAF_LIST_NODE ||
        node.get_type() == subdivision_node::DYNAMIC_ENTITY_LIST_NODE) {
        const auto &list = static_cast<const dynamic_entity_list_node &>(node);
        int result = 0;
        for (auto *entry = list.head; entry != nullptr; entry = entry->next) {
            const int status = visitor.visit(*reinterpret_cast<const subdivision_node *>(entry->ent));
            if (status == 3)
                return 3;
            if (status != 0)
                result = 2;
        }
        return result;
    }
    if (node.get_type() == subdivision_node::STATIC_LEGO_LIST_NODE)
        return static_lego_list_methods::traverse_all(node, visitor);
    const auto &list = static_cast<const static_region_list_node &>(node);
    assert(node.get_type() == subdivision_node::STATIC_REGION_LIST_NODE ||
           node.get_type() == subdivision_node::STATIC_LEAF_LIST_NODE);
    int result = 0;
    for (uint8_t index = 0; index < list.count; ++index) {
        const uint16_t id = list.region_indices()[index];
        if (node.get_type() == subdivision_node::STATIC_REGION_LIST_NODE) {
            auto &visited = static_region_list_methods::scratchpad()[id >> 5];
            const uint32_t bit = uint32_t{1} << (id & 31);
            if ((visited & bit) != 0)
                continue;
            visited |= bit;
            const auto &mirror = static_region_list_methods::mirror()[id];
            if (test != nullptr && test->field_0 != 2) {
                const auto &point = test->field_4;
                const float radius = test->field_0 == 0 ? test->field_10 : 0.0f;
                if (point.x > mirror.field_10.x + radius || point.x < mirror.field_4.x - radius ||
                    point.y > mirror.field_10.y + radius || point.y < mirror.field_4.y - radius ||
                    point.z > mirror.field_10.z + radius || point.z < mirror.field_4.z - radius)
                    continue;
            }
            if (visitor.visit(*reinterpret_cast<const subdivision_node *>(mirror.field_0)) == 3)
                return 2;
        } else {
            const auto &leaf = *reinterpret_cast<const subdivision_node *>(reinterpret_cast<const uint32_t *>(&list) +
                                                                           static_cast<int16_t>(id));
            const int status = visitor.visit(leaf);
            if (status == 1)
                return 0;
            if (status == 3)
                return 3;
            if (status != 0)
                result = 2;
        }
    }
    return result;
}

bool proximity_map::is_initialized()
{
    return this->initialized;
}


proximity_map_construction_leaf::proximity_map_construction_leaf(region *a2, const vector3d &a1, const vector3d &a4)
{
    this->field_0.r = a2;
    this->field_4 = a1;
    this->field_10 = a4;
}
