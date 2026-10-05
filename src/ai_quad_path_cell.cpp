#include "ai_quad_path_cell.h"

#include "common.h"
#include "func_wrapper.h"
#include "collide.h"
#include <algorithm>
#include <cfloat>
#include <cmath>

VALIDATE_SIZE(ai_quad_path_cell, 0x50);

ai_quad_path_cell::ai_quad_path_cell() {}

vector3d ai_quad_path_cell::get_edge_midpoint(ai_quad_path_cell *other)
{
    auto find_edge = [](const ai_quad_path_cell *cell, const ai_quad_path_cell *neighbor) {
        for (int edge = 0; edge < 4; ++edge)
            for (int index = 0; index < cell->neighbor_counts[edge]; ++index)
                if (cell->neighbors[edge][index] == neighbor)
                    return edge;
        return -1;
    };
    const int edge = find_edge(this, other);
    const int other_edge = find_edge(other, this);
    vector3d endpoints[4] = {field_0[edge], field_0[(edge + 1) % 4],
        other->field_0[other_edge], other->field_0[(other_edge + 1) % 4]};
    const auto delta = endpoints[1] - endpoints[0];
    int axis = 0;
    if (std::fabs(delta.y) > std::fabs(delta.x))
        axis = 1;
    if (std::fabs(delta.z) > std::fabs((&delta.x)[axis]))
        axis = 2;
    for (int first = 0; first < 4; ++first)
        for (int second = first + 1; second < 4; ++second)
            if ((&endpoints[first].x)[axis] >= (&endpoints[second].x)[axis])
                std::swap(endpoints[first], endpoints[second]);
    return (endpoints[1] + endpoints[2]) * 0.5f;
}

vector3d ai_quad_path_cell::get_midpoint() const
{
    return (field_0[0] + field_0[1] + field_0[2] + field_0[3]) * 0.25f;
}

ai_quad_path_cell *ai_quad_path_cell::get_edge_neighbor(int edge, int index) const
{
    return neighbors[edge][index];
}

bool ai_quad_path_cell::is_point_in_cell(const vector3d &position, float radius) const
{
    vector3d minimum = field_0[0], maximum = field_0[0];
    for (int index = 1; index < 4; ++index) {
        for (int axis = 0; axis < 3; ++axis) {
            (&minimum.x)[axis] = std::min((&minimum.x)[axis], (&field_0[index].x)[axis]);
            (&maximum.x)[axis] = std::max((&maximum.x)[axis], (&field_0[index].x)[axis]);
        }
    }
    const vector3d outside{
        std::max(position.x - maximum.x, minimum.x - position.x),
        std::max(position.y - maximum.y, minimum.y - position.y),
        std::max(position.z - maximum.z, minimum.z - position.z)};
    int major = 0;
    if (field_4C == 4 || field_4C == 8)
        major = 1;
    else if (field_4C == 16 || field_4C == 32)
        major = 2;
    for (int axis = 0; axis < 3; ++axis)
        if ((&outside.x)[axis] >= (axis == major ? radius : 0.0f))
            return false;
    auto rotated = [major](const vector3d &delta) {
        if (major == 1)
            return vector3d{delta.z, 0.0f, -delta.x};
        if (major == 2)
            return vector3d{delta.y, -delta.x, 0.0f};
        return vector3d{0.0f, delta.z, -delta.y};
    };
    const auto first = rotated(position - field_0[1]);
    const auto second = rotated(position - field_0[3]);
    auto dot = [](const vector3d &a, const vector3d &b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    };
    return dot(first, field_0[1] - field_0[0]) >= 0.0f
        && dot(first, field_0[2] - field_0[1]) >= 0.0f
        && dot(second, field_0[3] - field_0[2]) >= 0.0f
        && dot(second, field_0[0] - field_0[3]) >= 0.0f;
}

float ai_quad_path_cell::is_point_near_cell(const vector3d &position) const
{
    int major = 2, first_axis = 0, second_axis = 1;
    if (field_4C == 1 || field_4C == 2) {
        major = 0;
        first_axis = 1;
        second_axis = 2;
    } else if (field_4C == 4 || field_4C == 8) {
        major = 1;
        second_axis = 2;
    }
    bool inside = true;
    for (int edge = 0; edge < 4; ++edge) {
        const auto delta = field_0[(edge + 1) % 4] - field_0[edge];
        const auto relative = position - field_0[edge];
        if ((&delta.x)[first_axis] * (&relative.x)[second_axis]
            - (&delta.x)[second_axis] * (&relative.x)[first_axis] < 0.0f)
            inside = false;
    }
    if (inside) {
        const auto midpoint = get_midpoint();
        const float delta = (&position.x)[major] - (&midpoint.x)[major];
        return delta * delta;
    }
    float distance = FLT_MAX;
    for (const auto &vertex : field_0)
        distance = std::min<float>(distance, (position - vertex).length2());
    return distance;
}

vector3d ai_quad_path_cell::closest_point(const vector3d &position) const
{
    float distance = FLT_MAX;
    vector3d result;
    for (int edge = 0; edge < 4; ++edge) {
        vector3d projected;
        closest_point_segment(position, field_0[edge], field_0[(edge + 1) % 4], projected);
        const float candidate = (position - projected).length2();
        if (candidate < distance) {
            distance = candidate;
            result = projected;
        }
    }
    return result;
}

void ai_quad_path_cell::un_mash(void *a2, int a3, int a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        const auto buffer = reinterpret_cast<std::uintptr_t>(a2);
        for (int edge = 0; edge < 4; ++edge) {
            if (neighbor_counts[edge] == 0) {
                neighbors[edge] = nullptr;
                continue;
            }
            neighbors[edge] = reinterpret_cast<ai_quad_path_cell **>(
                buffer + a4 + reinterpret_cast<std::uintptr_t>(neighbors[edge]));
            for (int index = 0; index < neighbor_counts[edge]; ++index)
                neighbors[edge][index] = reinterpret_cast<ai_quad_path_cell *>(
                    buffer + a3 + 0x50u * reinterpret_cast<std::uintptr_t>(neighbors[edge][index]));
        }
        field_44 = 0;
    } else {
        THISCALL(0x00452C60, this, a2, a3, a4);
    }
}
