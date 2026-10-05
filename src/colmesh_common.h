#pragma once

#include "vector3d.h"
#include <cstddef>
#include <cstdint>

struct collision_triangle_indices {
    uint16_t vertex[3];
};


struct collision_triangle_list {
    uint16_t vertex_count;
    uint16_t triangle_count;
    vector3d vertices[1];

    const collision_triangle_indices *triangles() const
    {
        return reinterpret_cast<const collision_triangle_indices *>(vertices + vertex_count);
    }
};

struct collision_obb_t {
    vector3d field_0;
    float field_C;
    vector3d field_10;
    vector3d axis_y;
    vector3d axis_z;
    bool field_34;
    uint8_t field_35;
    uint16_t field_36;

    const collision_triangle_list &triangles() const
    {
        const uint32_t offset = (uint32_t(field_35) << 16) | field_36;
        return *reinterpret_cast<const collision_triangle_list *>(
            reinterpret_cast<const unsigned char *>(this) + offset);
    }
};

static_assert(sizeof(collision_obb_t) == 0x38);
static_assert(offsetof(collision_obb_t, field_34) == 0x34);
static_assert(offsetof(collision_triangle_list, vertices) == 4);
static_assert(sizeof(collision_triangle_indices) == 6);

bool segment_mesh_box_overlap(const vector3d &start, const vector3d &end,
    const collision_obb_t &box);
