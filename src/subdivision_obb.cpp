#include "subdivision_obb.h"

#include "capsule.h"
#include "collide_aux.h"
#include "local_collision.h"
#include "common.h"
#include "func_wrapper.h"
#include "osassert.h"
#include "utility.h"
#include "vector4d.h"

#include <cassert>
#include <algorithm>
#include <cmath>

VALIDATE_SIZE(subdivision_node_obb_base, 0x16);

VALIDATE_SIZE(subdivision_node_aabb, 0x1C);
VALIDATE_OFFSET(subdivision_node_aabb, m_size, 0x16);

VALIDATE_SIZE(subdivision_node_obb, 0x2E);
VALIDATE_OFFSET(subdivision_node_obb, x_length, 0x28);

VALIDATE_SIZE(subdivision_node_large_aabb, 0x24);
VALIDATE_OFFSET(subdivision_node_large_aabb, m_size, 0x18);

VALIDATE_SIZE(subdivision_node_large_obb, 0x48);
VALIDATE_OFFSET(subdivision_node_large_obb, x_axis, 0x18);
VALIDATE_OFFSET(subdivision_node_large_obb, x_length, 0x3C);

subdivision_node_obb_base::subdivision_node_obb_base() {}

void subdivision_node_obb_base::init(uint16_t flags_arg)
{
    this->flags = flags_arg;
    memset(this->terrain_type_info, 0, 3);
    this->center = ZEROVEC;
    this->set_type(UNDEFINED_NODE);
}

bool subdivision_node_obb_base::point_inside_or_on(const vector3d &a2) const
{
    if constexpr (1) {
        vector4d v15, v24, a4, a6;

        auto v3 = this->unpack_xform(v15, v24, a4, a6);
        auto v4 = this->center[1];

        float v13[4]{};
        v13[0] = this->center[0];
        auto v5 = this->center[2];
        v13[1] = v4;
        v13[2] = v5;

        vector4d v20;
        v20[1] = v4;
        v20[0] = v13[0];
        v20[3] = v13[3];
        v20[2] = v5;

        vector4d a3{};
        a3[0] = a2[0];
        auto v6 = a2[2];
        a3[1] = a2[1];
        v13[0] = a3[0];
        a3[0] = a3[0] - v20[0];
        v13[1] = a3[1];
        v13[2] = v6;
        v13[3] = a3[3];
        a3[1] = a3[1] - v4;
        a3[2] = v6 - v20[2];
        a3[3] = a3[3] - v20[3];

        if (v3) {
            auto v7 = sub_4126E0(v24, a3, a4, a3, a6, a3);
            auto v8 = v7[1];
            a3[0] = v7[0];
            auto v9 = v7[2];
            a3[1] = v8;
            auto v10 = v7[3];
            a3[2] = v9;
            a3[3] = v10;
        }

        return v15[0] >= std::abs(a3[0]) && v15[1] >= std::abs(a3[1]) && v15[2] >= std::abs(a3[2]);

    } else {
        return (bool)THISCALL(0x0052BD30, this, &a2);
    }
}

void subdivision_node_obb_base::get_extents(vector3d *min_extent, vector3d *max_extent)
{
    assert(min_extent != nullptr);
    assert(max_extent != nullptr);
    vector3d vertices[8];
    get_vertices(vertices);
    *min_extent = vertices[0];
    *max_extent = vertices[0];
    for (int index = 1; index < 8; ++index) {
        *min_extent = vector3d::min(*min_extent, vertices[index]);
        *max_extent = vector3d::max(*max_extent, vertices[index]);
    }
}

void subdivision_node_obb_base::get_vertices(vector3d *out) const
{
    vector3d axes[3];
    unpack_axii(axes);
    for (int index = 0; index < 8; ++index) {
        const float sx = (index & 2) ? -1.0f : 1.0f;
        const float sy = (index & 4) ? -1.0f : 1.0f;
        const float sz = ((index ^ (index >> 1)) & 1) ? -1.0f : 1.0f;
        out[index] = center + axes[0] * sx + axes[1] * sy + axes[2] * sz;
    }
}

float *subdivision_node_obb_base::sub_564D50(float *out)
{
    vector4d half{}, row_x{}, row_y{}, row_z{};
    unpack_xform(half, row_x, row_y, row_z);
    out[0] = half[0];
    out[1] = half[1];
    out[2] = half[2];
    out[3] = 0.0f;
    return out;
}

float subdivision_node_obb_base::sub_52CA80()
{
    float v2[4];

    this->sub_564D50(v2);
    return v2[0] * v2[1] * v2[2] * 8.0f;
}

bool subdivision_node_obb_base::unpack_xform(vector4d &half, vector4d &row_x,
                                            vector4d &row_y, vector4d &row_z) const
{
    constexpr double length_scale = double(0.0010681315f);
    constexpr double axis_scale = double(0.000030517578f);
    half = vector4d{};
    row_x = vector4d{};
    row_y = vector4d{};
    row_z = vector4d{};
    switch (get_type()) {
    case AABB_LEAF_NODE: {
        const auto &box = static_cast<const subdivision_node_aabb &>(*this);
        half[0] = box.m_size.x * length_scale;
        half[1] = box.m_size.y * length_scale;
        half[2] = box.m_size.z * length_scale;
        return false;
    }
    case AABB_LARGE_LEAF_NODE: {
        const auto &box = static_cast<const subdivision_node_large_aabb &>(*this);
        half[0] = box.m_size.x;
        half[1] = box.m_size.y;
        half[2] = box.m_size.z;
        return false;
    }
    case OBB_LEAF_NODE: {
        const auto &box = static_cast<const subdivision_node_obb &>(*this);
        row_x[0] = box.x_axis.x * axis_scale;
        row_x[1] = box.y_axis.x * axis_scale;
        row_x[2] = box.z_axis.x * axis_scale;
        row_y[0] = box.x_axis.y * axis_scale;
        row_y[1] = box.y_axis.y * axis_scale;
        row_y[2] = box.z_axis.y * axis_scale;
        row_z[0] = box.x_axis.z * axis_scale;
        row_z[1] = box.y_axis.z * axis_scale;
        row_z[2] = box.z_axis.z * axis_scale;
        half[0] = box.x_length * length_scale;
        half[1] = box.y_length * length_scale;
        half[2] = box.z_length * length_scale;
        return true;
    }
    case OBB_LARGE_LEAF_NODE:
    case AUDIO_OBB_LEAF_NODE: {
        const auto &box = static_cast<const subdivision_node_large_obb &>(*this);
        row_x[0] = box.x_axis.x;
        row_x[1] = box.y_axis.x;
        row_x[2] = box.z_axis.x;
        row_y[0] = box.x_axis.y;
        row_y[1] = box.y_axis.y;
        row_y[2] = box.z_axis.y;
        row_z[0] = box.x_axis.z;
        row_z[1] = box.y_axis.z;
        row_z[2] = box.z_axis.z;
        half[0] = box.x_length;
        half[1] = box.y_length;
        half[2] = box.z_length;
        return true;
    }
    default:
        return false;
    }
}


bool collision_segment_box_overlap(const vector3d &to_center, const vector3d &from_center,
                                   const vector3d &half)
{
    for (int axis = 0; axis != 3; ++axis) {
        if (half[axis] - std::abs(to_center[axis]) < 0.0f &&
            half[axis] - std::abs(from_center[axis]) < 0.0f &&
            to_center[axis] * from_center[axis] < 0.0f)
            return false;
    }
    const vector3d direction = to_center + from_center;
    for (int axis = 0; axis != 3; ++axis) {
        const int next = (axis + 1) % 3;
        const float cross = std::abs(to_center[axis] * direction[next] -
                                     to_center[next] * direction[axis]);
        if (half[axis] * std::abs(direction[next]) +
                half[next] * std::abs(direction[axis]) - cross < 0.0f)
            return false;
    }
    return true;
}

namespace {
vector3d obb_local_vector(const vector3d &v, const vector4d &row_x,
                          const vector4d &row_y, const vector4d &row_z)
{
    return {row_x[0] * v.x + row_y[0] * v.y + row_z[0] * v.z,
            row_x[1] * v.x + row_y[1] * v.y + row_z[1] * v.z,
            row_x[2] * v.x + row_y[2] * v.y + row_z[2] * v.z};
}

bool inside_box(const vector3d &v, const vector3d &half)
{
    return half.x >= std::abs(v.x) && half.y >= std::abs(v.y) && half.z >= std::abs(v.z);
}
}

bool subdivision_node_obb_base::line_segment_intersection(const vector3d &start, const vector3d &end)
{
    if ((flags & 0x101) != 0)
        return false;
    vector4d extent, row_x, row_y, row_z;
    const bool rotated = unpack_xform(extent, row_x, row_y, row_z);
    vector3d to_center = center - start;
    vector3d from_center = end - center;
    if (rotated) {
        to_center = obb_local_vector(to_center, row_x, row_y, row_z);
        from_center = obb_local_vector(from_center, row_x, row_y, row_z);
    }
    const vector3d half{extent[0], extent[1], extent[2]};

    return collision_segment_box_overlap(to_center, from_center, half) && !inside_box(to_center, half);
}

bool subdivision_node_obb_base::line_segment_intersection(const vector3d &start, const vector3d &end,
                                                          vector3d *point, vector3d *normal,
                                                          float *fraction, bool allow_exit)
{
    if ((flags & 0x101) != 0)
        return false;
    vector4d extent, row_x, row_y, row_z;
    const bool rotated = unpack_xform(extent, row_x, row_y, row_z);
    vector3d to_center = center - start;
    vector3d from_center = end - center;
    if (rotated) {
        to_center = obb_local_vector(to_center, row_x, row_y, row_z);
        from_center = obb_local_vector(from_center, row_x, row_y, row_z);
    }
    const vector3d half{extent[0], extent[1], extent[2]};
    if (!collision_segment_box_overlap(to_center, from_center, half))
        return false;
    const bool exiting = inside_box(to_center, half);
    if (exiting && (!allow_exit || inside_box(from_center, half)))
        return false;



    constexpr float epsilon = 0.00009999999747378752f;
    constexpr float largest = 3.402823466e38f;
    const vector3d direction = to_center + from_center;
    vector3d parameters;
    for (int axis = 0; axis != 3; ++axis) {
        const float reciprocal = 1.0f / direction[axis];
        const float offset = to_center[axis] * reciprocal;
        const float span = half[axis] * std::abs(reciprocal);
        parameters[axis] = std::abs(direction[axis]) < epsilon
            ? (exiting ? largest : -largest)
            : (exiting ? offset + span : offset - span);
    }
    int axis;
    if (exiting) {
        if (parameters.x <= parameters.y && parameters.x <= parameters.z) {
            if (parameters.x > 1.0f)
                return false;
            axis = 0;
        } else {
            axis = parameters.y <= parameters.z ? 1 : 2;
        }
    } else {
        if (parameters.x < parameters.y)
            axis = parameters.y >= parameters.z ? 1 : 2;
        else if (parameters.x < parameters.z)
            axis = 2;
        else {
            if (parameters.x < 0.0f)
                return false;
            axis = 0;
        }
    }
    const float t = parameters[axis];
    if (fraction)
        *fraction = t;
    *point = start + end * t - start * t;
    if (rotated)
        *normal = {row_x[axis], row_y[axis], row_z[axis]};
    else {
        *normal = vector3d{};
        (*normal)[axis] = 1.0f;
    }
    if (exiting ? direction[axis] < 0.0f : direction[axis] > 0.0f)
        *normal = -*normal;
    return true;
}

bool subdivision_node_obb_base::sphere_intersection(const vector3d &sphere_center, Float radius,
                                                    vector3d *point, vector3d *normal, float *separation)
{
    if ((flags & 0x101) != 0)
        return false;
    vector4d extent, row_x, row_y, row_z;
    const bool rotated = unpack_xform(extent, row_x, row_y, row_z);
    vector3d local = sphere_center - center;
    if (rotated)
        local = obb_local_vector(local, row_x, row_y, row_z);
    const vector3d half{extent.x, extent.y, extent.z};
    vector3d closest;
    vector3d absolute{std::abs(local.x), std::abs(local.y), std::abs(local.z)};
    vector3d clamped = vector3d::min(half, absolute);
    const float distance_squared = (absolute - clamped).length2();
    if (distance_squared > radius.value * radius.value)
        return false;
    const float distance = std::sqrt(distance_squared);
    vector3d local_normal;
    if (distance > 0.0f) {
        for (int axis = 0; axis < 3; ++axis)
            closest[axis] = local[axis] < 0.0f ? -clamped[axis] : clamped[axis];
        local_normal = (local - closest) / distance;
        if (separation)
            *separation = distance - radius.value;
    } else {


        const vector3d remaining = half - clamped;
        for (int axis = 0; axis < 3; ++axis) {
            const float other = std::min(remaining[(axis + 1) % 3], remaining[(axis + 2) % 3]);
            const bool nearest = !(other - remaining[axis] < 0.0f);
            const float sign = local[axis] < 0.0f ? -1.0f : 1.0f;
            closest[axis] = sign * (nearest ? half[axis] : clamped[axis]);
            local_normal[axis] = nearest ? sign : 0.0f;
        }
        local_normal.normalize();
        if (separation)
            *separation = dot(local - closest, local_normal) - radius.value;
    }
    if (rotated) {
        *point = center + vector3d{
            row_x.x * closest.x + row_x.y * closest.y + row_x.z * closest.z,
            row_y.x * closest.x + row_y.y * closest.y + row_y.z * closest.z,
            row_z.x * closest.x + row_z.y * closest.y + row_z.z * closest.z};
        *normal = vector3d{
            row_x.x * local_normal.x + row_x.y * local_normal.y + row_x.z * local_normal.z,
            row_y.x * local_normal.x + row_y.y * local_normal.y + row_y.z * local_normal.z,
            row_z.x * local_normal.x + row_z.y * local_normal.y + row_z.z * local_normal.z};
    } else {
        *point = center + closest;
        *normal = local_normal;
    }
    return true;
}

bool subdivision_node_obb_base::sphere_intersection(const vector3d &sphere_center, Float radius)
{
    if ((flags & 0x101) != 0)
        return false;
    vector4d extent, row_x, row_y, row_z;
    const bool rotated = unpack_xform(extent, row_x, row_y, row_z);
    vector3d local = sphere_center - center;
    if (rotated)
        local = obb_local_vector(local, row_x, row_y, row_z);
    const vector3d absolute{std::abs(local.x), std::abs(local.y), std::abs(local.z)};
    const vector3d clamped = vector3d::min(vector3d{extent.x, extent.y, extent.z}, absolute);
    return (absolute - clamped).length2() <= radius.value * radius.value;
}

bool subdivision_node_obb_base::capsule_intersection(
    const capsule &query, local_collision::closest_points_pair_t *pair)
{
    if ((flags & 0x101) != 0)
        return false;
    const vector3d midpoint = (query.base + query.end) * 0.5f;
    if (!sphere_intersection(midpoint, (query.end - query.base).length() * 0.5f + query.radius))
        return false;

    vector3d axes[3];
    unpack_axii(axes);
    const vector3d start = query.base - center;
    const vector3d delta = query.end - query.base;
    float coordinates[3], derivatives[3], lengths_squared[3];
    int regions[3];
    struct boundary { float time; int step; int axis; };
    boundary boundaries[7]{{1.0f, 0, 0}};
    int count = 1;
    for (int axis = 0; axis != 3; ++axis) {
        lengths_squared[axis] = axes[axis].length2();
        coordinates[axis] = dot(axes[axis], start);
        derivatives[axis] = dot(axes[axis], delta);
        const float coordinate = coordinates[axis] / lengths_squared[axis];
        regions[axis] = coordinate > 1.0f ? 1 : coordinate < -1.0f ? -1 : 0;
        if (std::abs(derivatives[axis]) > 0.000099999997f) {
            const int step = derivatives[axis] > 0.0f ? 1 : -1;
            for (int side : {1, -1}) {
                const float time = (side * lengths_squared[axis] - coordinates[axis]) / derivatives[axis];
                if (time > 0.0f && time < 1.0f)
                    boundaries[count++] = {time, step, axis};
            }
        }
    }
    std::sort(boundaries, boundaries + count,
        [](const boundary &a, const boundary &b) { return a.time < b.time; });
    vector3d interval_start = start;
    vector3d best_axis, best_box, best_normal;
    float best_distance = 3.402823466e38f;
    for (int index = 0; index != count; ++index) {
        const vector3d interval_end = start + delta * boundaries[index].time;
        vector3d fixed_axes[3], free_axis;
        int fixed_count = 0;
        for (int axis = 0; axis != 3; ++axis) {
            if (regions[axis])
                fixed_axes[fixed_count++] = axes[axis] * float(regions[axis]);
            else
                free_axis = axes[axis];
        }
        vector3d axis_point, box_point, normal;
        switch (fixed_count) {
        case 0: {


            float largest = -1.0f;
            int selected_axis = -1;
            vector3d selected_coordinates;
            for (int axis = 0; axis != 3; ++axis) {
                for (int endpoint = 0; endpoint != 2; ++endpoint) {
                    const float coordinate = (coordinates[axis] + derivatives[axis] * endpoint) /
                                             lengths_squared[axis];
                    if (std::abs(coordinate) >= largest) {
                        largest = std::abs(coordinate);
                        selected_axis = axis;
                        axis_point = endpoint ? interval_end : interval_start;
                        for (int dimension = 0; dimension != 3; ++dimension)
                            selected_coordinates[dimension] =
                                (coordinates[dimension] + derivatives[dimension] * endpoint) /
                                lengths_squared[dimension];
                    }
                }
            }
            selected_coordinates[selected_axis] = selected_coordinates[selected_axis] >= 0.0f ? 1.0f : -1.0f;
            box_point = axes[0] * selected_coordinates.x + axes[1] * selected_coordinates.y +
                        axes[2] * selected_coordinates.z;
            normal = axes[selected_axis];
            break;
        }
        case 1:
            closest_point_line_segment_plane(interval_start, interval_end, fixed_axes[0], fixed_axes[0],
                &axis_point, &box_point);
            normal = fixed_axes[0];
            break;
        case 2: {
            const vector3d edge_center = fixed_axes[0] + fixed_axes[1];
            const vector3d edge_start = edge_center + free_axis;
            const vector3d edge_end = edge_center - free_axis;
            float axis_time, edge_time;
            closest_point_line_segment_line_segment(interval_start, interval_end, edge_start, edge_end,
                &axis_time, &edge_time);
            axis_point = interval_start + (interval_end - interval_start) * axis_time;
            box_point = edge_start + (edge_end - edge_start) * edge_time;
            normal = axis_point - box_point;
            break;
        }
        case 3: {
            box_point = fixed_axes[0] + fixed_axes[1] + fixed_axes[2];
            float time;
            closest_point_line_segment_point(interval_start, interval_end, box_point, time);
            axis_point = interval_start + (interval_end - interval_start) * time;
            normal = axis_point - box_point;
            break;
        }
        }
        const float distance = (axis_point - box_point).length2();
        if (distance < best_distance) {
            best_distance = distance;
            best_axis = axis_point;
            best_box = box_point;
            best_normal = normal;
        }
        interval_start = interval_end;
        regions[boundaries[index].axis] += boundaries[index].step;
    }
    if (pair) {
        vector3d direction = best_box - best_axis;
        if (direction.length2() > 9.999999439624929e-11f)
            direction /= std::sqrt(direction.length2());
        if (best_normal.length2() > 9.999999439624929e-11f)
            best_normal /= std::sqrt(best_normal.length2());
        pair->point = best_axis + center;
        pair->other_point = best_box + center;
        pair->direction = direction;
        pair->normal = best_normal;
        pair->distance_squared = best_distance;
    }
    return best_distance < query.radius * query.radius;
}

bool subdivision_node_obb_base::find_closest_point_on_visible_faces(const vector3d &sweet_spot, const vector3d &ent_pos,
                                                                    fixed_vector<obb_closest_point_entry_t, 3> *results)
{
    assert(results != nullptr);
    assert(results->size() == 0);

    if constexpr (0) {
#if 0
        
        if ((this->flags & 0x101) != 0) {
            return false;
        }

        vector4d v64, a2, a4, a6;
        auto v6 = this->unpack_xform(v64, a2, a4, a6);
        auto v7 = this->center[0];
        auto v8 = this->center[1];
        auto v9 = v6;
        auto v10 = !v6;

        vector4d a8;
        a8[2] = this->center[2];

        vector4d v59{};
        v59[2] = a8[2];
        a8[0] = v7;
        v59[0] = v7;
        a8[1] = v8;
        v59[1] = v8;

        auto v11 = sweet_spot[0];
        a8[3] = v59[3];

        auto v12 = sweet_spot[1];
        v59[0] = v11;

        auto v13 = sweet_spot[2];

        vector4d v62{};
        v62[0] = v59[0];
        v62[1] = v12;
        v62[2] = v13;

        vector4d a3{};
        a3[0] = v59[0] - a8[0];
        v62[3] = v59[3];
        v59[1] = v12;
        v62[0] = ent_pos[0];
        a3[1] = v12 - a8[1];
        v59[2] = v13;

        auto v14 = ent_pos[2];
        v62[1] = ent_pos[1];
        v62[2] = v14;
        a3[2] = v13 - a8[2];
        v59[0] = v62[0];
        v62[3] = v59[3];

        char v57[19]{};
        v57[0] = v9;

        v59[1] = v62[1];
        a3[3] = v59[3] - v59[3];
        v59[2] = v14;
        auto a5 = v62[0] - a8[0];
        auto v68 = v62[1] - a8[1];
        auto v69 = v14 - a8[2];
        auto v70 = a3[3];

        vector4d arg4a{};
        vector4d v72{};
        if (!v10) {
            auto v15 = vector4d::sub_4126E0(a2, a3.arr, a4, a3.arr, a6, a3.arr);
            auto v16 = v15[1];
            a3[0] = v15[0];
            auto v17 = v15[2];
            a3[1] = v16;
            auto v18 = v15[3];
            a3[2] = v17;
            a3[3] = v18;

            auto v19 = vector4d::sub_4126E0(a2, &a5, a4, &a5, a6, &a5);
            auto v20 = v19[1];
            a5 = v19[0];
            auto v21 = v19[2];
            v68 = v20;
            auto v22 = v19[3];
            v69 = v21;
            v70 = v22;
            arg4a[0] = a2[0];
            arg4a[1] = a4[0];
            arg4a[2] = a6[0];
            v72[0] = a2[1];
            v72[1] = a4[1];
            v72[2] = a6[1];

            v62[0] = a2[2];
            v62[1] = a4[2];
            v62[2] = a6[2];
        }

        vector4d a1{};
        a1[0] = v64[0] - std::abs(a5);
        a1[1] = v64[1] - std::abs(v68);
        a1[2] = v64[2] - std::abs(v69);
        a2[0] = -v64[0];
        a2[1] = -v64[1];
        a2[2] = -v64[2];
        a2[3] = -v64[3];

        auto v23 = vector4d::min(v64, a3);
        a3 = vector4d::max(a2, v23);
        a2 = sub_55DA40(&a5, &v64);
        int v24 = 0;
        v57[1] = a1[0] < 0.0f;
        v57[2] = a1[1] < 0.0f;
        if (a1[0] < 0.0f) {
            *(float *) &v57[7] = a3[1];
            *(float *) &v57[11] = a3[2];
            *(float *) &v57[15] = a3[3];
            *(float *) &v57[3] = a2[0];
            v64[0] = a2[0];
            v64[1] = a3[1];
            v64[2] = a3[2];
            v64[3] = a3[3];
            if (v9) {
                v25 = sub_413E90((math::VecClass__3_1 *) &v59,
                                 &arg4a,
                                 (float *) &v57[3],
                                 &v72,
                                 (float *) &v57[3],
                                 &v62,
                                 (float *) &v57[3],
                                 &a8);
                v26 = v25->field_0[1];
                v27 = v25->field_0[2];
                *(float *) &v57[3] = v25->field_0[0];
                v60 = arg4a;
            } else {
                sub_4119B0((float *) &v57[3], a8.base.arr);
                sub_56A8E0(&v60, (int) v57);
                v27 = *(float *) &v57[11];
                v26 = *(float *) &v57[7];
            }
            if (a5 < (double) float_NULL) {
                v28 = sub_5610A0(v59.base.arr, v60.base.arr);
                v29 = v28[1];
                v60.base.arr[0] = *v28;
                v30 = v28[2];
                v31 = v28[3];
                v60.base.arr[1] = v29;
                v60.base.arr[2] = v30;
                v60.field_C = v31;
            }
            results->m_data[0].field_0.arr[0] = *(float *) &v57[3];
            v32 = v60.base.arr[0];
            results->m_data[0].field_0.arr[2] = v27;
            v33 = v60.base.arr[2];
            results->m_data[0].field_0.arr[1] = v26;
            v59.base.arr[0] = v32;
            v9 = v57[0];
            v59.base.arr[1] = v60.base.arr[1];
            v59.base.arr[2] = v33;
            v34 = v60.base.arr[1];
            v59.field_C = v60.field_C;
            v35 = v59.base.arr[2];
            results->m_data[0].field_C.arr[0] = v59.base.arr[0];
            results->m_data[0].field_C.arr[1] = v34;
            results->m_data[0].field_C.arr[2] = v35;
            v24 = 1;
        }
        if (v57[2]) {
            *(float *) &v57[7] = a2.base.arr[1];
            *(float *) &v57[3] = a3.base.arr[0];
            *(float *) &v57[11] = a3.base.arr[2];
            *(float *) &v57[15] = a3.field_C;
            if (v57[1] &&
                (a4.base.arr[0] = v64.base.arr[0] - *(float *) &v57[3],
                 a4.base.arr[1] = v64.base.arr[1] - a2.base.arr[1],
                 a4.base.arr[2] = v64.base.arr[2] - *(float *) &v57[11],
                 a4.field_C = v64.field_C - *(float *) &v57[15],
                 a6 = a4,
                 a4.base.arr[0] * a4.base.arr[0] + a4.base.arr[2] * a4.base.arr[2] +
                         a4.base.arr[1] * a4.base.arr[1] <=
                     LARGE_EPSILON)) {
                v57[2] = 0;
            } else {
                v59.base.arr[0] = a3.base.arr[0];
                v59.base.arr[1] = *(float *) &v57[7];
                v59.base.arr[2] = a3.base.arr[2];
                v59.field_C = a3.field_C;
                if (v9) {
                    v36 = sub_413E90((math::VecClass__3_1 *) &a4,
                                     &arg4a,
                                     (float *) &v57[3],
                                     &v72,
                                     (float *) &v57[3],
                                     &v62,
                                     (float *) &v57[3],
                                     &a8);
                    v37 = v36->field_C;
                    v38 = v36->field_0[1];
                    v39 = v36->field_0[2];
                    *(float *) &v57[3] = v36->field_0[0];
                    *(float *) &v57[15] = v37;
                    v60 = v72;
                } else {
                    sub_4119B0((float *) &v57[3], a8.base.arr);
                    sub_56A9D0(&v60, (int) v57);
                    v39 = *(float *) &v57[11];
                    v38 = *(float *) &v57[7];
                }
                if (v68 < (double) float_NULL) {
                    v40 = sub_5610A0(a4.base.arr, v60.base.arr);
                    v41 = v40[1];
                    v60.base.arr[0] = *v40;
                    v42 = v40[2];
                    v43 = v40[3];
                    v60.base.arr[1] = v41;
                    v60.base.arr[2] = v42;
                    v60.field_C = v43;
                }
                v44 = results->m_data[v24].field_0.arr;
                *v44 = *(float *) &v57[3];
                v44[1] = v38;
                v45 = v60.base.arr[0];
                v44[2] = v39;
                a4.base.arr[0] = v45;
                v9 = v57[0];
                a4.base.arr[1] = v60.base.arr[1];
                a4.base.arr[2] = v60.base.arr[2];
                a4.field_C = v60.field_C;
                v46 = v60.base.arr[1];
                v44[3] = v45;
                v47 = a4.base.arr[2];
                v44[4] = v46;
                v44[5] = v47;
                ++v24;
            }
        }
        if (a1.base.arr[2] < (double) float_NULL) {
            *(float *) &v57[7] = a3.base.arr[1];
            *(float *) &v57[11] = a2.base.arr[2];
            *(float *) &v57[3] = a3.base.arr[0];
            *(float *) &v57[15] = a3.field_C;
            if (!v57[1] ||
                (a4.base.arr[0] = v64.base.arr[0] - *(float *) &v57[3],
                 a4.base.arr[1] = v64.base.arr[1] - *(float *) &v57[7],
                 a4.base.arr[2] = v64.base.arr[2] - a2.base.arr[2],
                 a4.field_C = v64.field_C - *(float *) &v57[15],
                 a6 = a4,
                 a4.base.arr[0] * a4.base.arr[0] + a4.base.arr[2] * a4.base.arr[2] +
                         a4.base.arr[1] * a4.base.arr[1] >
                     LARGE_EPSILON)) {
                if (!v57[2] ||
                    (a4.base.arr[0] = v59.base.arr[0] - *(float *) &v57[3],
                     a4.base.arr[1] = v59.base.arr[1] - *(float *) &v57[7],
                     a4.base.arr[2] = v59.base.arr[2] - a2.base.arr[2],
                     a4.field_C = v59.field_C - *(float *) &v57[15],
                     a6 = a4,
                     a4.base.arr[2] * a4.base.arr[2] + a4.base.arr[1] * a4.base.arr[1] +
                             a4.base.arr[0] * a4.base.arr[0] >
                         LARGE_EPSILON)) {
                    if (v9) {
                        v48 = sub_413E90((math::VecClass__3_1 *) &a1,
                                         &arg4a,
                                         (float *) &v57[3],
                                         &v72,
                                         (float *) &v57[3],
                                         &v62,
                                         (float *) &v57[3],
                                         &a8);
                        v49 = v48->field_0[0];
                        v50 = v48->field_0[1];
                        v51 = v48->field_0[2];
                        v52 = v48->field_C;
                        *(float *) &v57[3] = v49;
                        *(float *) &v57[15] = v52;
                        v60 = v62;
                    } else {
                        sub_4119B0((float *) &v57[3], a8.base.arr);
                        sub_56AA20(&v60, (int) v57);
                        v51 = *(float *) &v57[11];
                        v50 = *(float *) &v57[7];
                    }
                    if (v69 < (double) float_NULL) {
                        v53 = sub_5610A0(a1.base.arr, v60.base.arr);
                        v54 = v53[1];
                        v60.base.arr[0] = *v53;
                        v55 = v53[2];
                        v60.base.arr[1] = v54;
                        v56 = v53[3];
                        v60.base.arr[2] = v55;
                        v60.field_C = v56;
                    }
                    a1.base.arr[0] = *(float *) &v57[3];
                    a1.base.arr[2] = v51;
                    a1.field_C = *(float *) &v57[15];
                    a1.base.arr[1] = v50;
                    sub_560B90(&results->m_data[v24].field_0, a1.base.arr);
                    sub_560B90(&results->m_data[v24++].field_C, v60.base.arr);
                }
            }
        }
        results->m_size = v24;
        return v24 > 0;

#endif

    } else {
        return THISCALL(0x005391F0, this, &sweet_spot, &ent_pos, results);
    }
}

bool subdivision_node_obb_base::is_obb_node() const
{
    return this->get_type() == 4 || this->get_type() == 5 || this->get_type() == 6 || this->get_type() == 7 ||
           this->get_type() == 8;
}

void subdivision_node_obb_base::unpack_axii(vector3d *axii) const
{
    if (get_type() == AABB_LEAF_NODE) {
        const auto &box = static_cast<const subdivision_node_aabb &>(*this);
        constexpr double scale = double(0.0010681315f);
        axii[0] = {static_cast<float>(box.m_size.x * scale), 0.0f, 0.0f};
        axii[1] = {0.0f, static_cast<float>(box.m_size.y * scale), 0.0f};
        axii[2] = {0.0f, 0.0f, static_cast<float>(box.m_size.z * scale)};
        return;
    }
    if (get_type() == OBB_LEAF_NODE) {
        const auto &box = static_cast<const subdivision_node_obb &>(*this);
        constexpr double length_scale = double(0.0010681315f);
        constexpr double axis_scale = double(0.000030517578f);
        const auto decode = [](const auto &axis, unsigned length) {
            const float extent = static_cast<float>(length * length_scale);
            const vector3d direction{
                static_cast<float>(axis.x * axis_scale),
                static_cast<float>(axis.y * axis_scale),
                static_cast<float>(axis.z * axis_scale)};
            return direction * extent;
        };
        axii[0] = decode(box.x_axis, box.x_length);
        axii[1] = decode(box.y_axis, box.y_length);
        axii[2] = decode(box.z_axis, box.z_length);
        return;
    }
    if (get_type() == AABB_LARGE_LEAF_NODE) {
        auto *self = static_cast<const subdivision_node_large_aabb *>(this);
        axii[0] = {self->m_size.x, 0.0f, 0.0f};
        axii[1] = {0.0f, self->m_size.y, 0.0f};
        axii[2] = {0.0f, 0.0f, self->m_size.z};
        return;
    }
    if (get_type() == OBB_LARGE_LEAF_NODE || get_type() == AUDIO_OBB_LEAF_NODE) {
        auto *self = static_cast<const subdivision_node_large_obb *>(this);
        axii[0] = self->x_axis * self->x_length;
        axii[1] = self->y_axis * self->y_length;
        axii[2] = self->z_axis * self->z_length;
        return;
    }
    assert(false && "unsupported subdivision node type");
}

subdivision_node_obb::subdivision_node_obb()
{
    set_type(OBB_LEAF_NODE);
}

subdivision_node_aabb::subdivision_node_aabb()
{
    this->set_type(AABB_LEAF_NODE);
}

void check_for_degeneracies(subdivision_node_obb_base *obb)
{
    vector3d a1[3]{};
    obb->unpack_axii(a1);

    for (int i = 0; i < 3; ++i) {
        if (dot(a1[i], a1[i]) <= 0.00019999999) {
            auto func = [](subdivision_node_obb_base *self) -> vector3d {
                return self->center;
            };

            auto v3 = func(obb)[2];
            auto v2 = func(obb)[1];
            auto v1 = func(obb)[0];
            error("Data error: degenerate (thin) obb found at (%.2f, %.2f, %.2f).\n"
                  "Please correct the obb in the MAX file corresponding to these coordinates.\n"
                  "(This error can be ignored more or less safely)",
                  v1,
                  v2,
                  v3);
        }
    }
}

bool subdivision_node_large_aabb::init(uint16_t a2, uint32_t terrain_type_info_arg, const vector3d &a4,
                                       const vector3d &a5)
{
    subdivision_node_obb_base::init(a2);

    this->set_type(AABB_LARGE_LEAF_NODE);
    this->center = a4;
    this->m_size = a5;

    assert(!(terrain_type_info_arg & 0xFF000000));

    std::memcpy(this->terrain_type_info, &terrain_type_info_arg, 3);

    check_for_degeneracies(this);

    return true;
}

bool subdivision_node_large_obb::init(uint16_t a2, uint32_t terrain_type_info_arg, const vector3d &a4,
                                      const vector3d &a5, const vector3d &a6, const vector3d &a7)
{
    subdivision_node_obb_base::init(a2);

    this->center = a4;
    this->x_axis = a5;
    this->y_axis = a6;
    this->z_axis = a7;

    this->x_length = this->x_axis.length();
    this->y_length = this->y_axis.length();
    this->z_length = this->z_axis.length();

    assert(x_length > EPSILON && y_length > EPSILON && z_length > EPSILON);

    this->x_axis /= this->x_length;

    this->y_axis /= this->y_length;

    this->z_axis /= this->z_length;

    assert(!(terrain_type_info_arg & 0xFF000000));

    std::memcpy(this->terrain_type_info, &terrain_type_info_arg, 3);

    check_for_degeneracies(bit_cast<subdivision_node_obb_base *>(this));

    return true;
}
