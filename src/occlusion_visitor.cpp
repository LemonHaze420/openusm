#include "occlusion_visitor.h"

#include "occlusion.h"
#include "subdivision_obb.h"
#include "vector4d.h"

#include <cmath>
#include <utility>

namespace {
int visit_occlusion_node(subdivision_visitor &visitor, const subdivision_node &node)
{
    return static_cast<occlusion_visitor &>(visitor).visit(node);
}

const subdivision_visitor::native_vtable occlusion_table{visit_occlusion_node, nullptr};

vector3d component_product(const vector3d &a, const vector3d &b)
{
    return {a.x * b.x, a.y * b.y, a.z * b.z};
}

double scalar_product(const vector3d &a, const vector3d &b)
{
    return double(a.z) * b.z + double(a.y) * b.y + double(a.x) * b.x;
}
}  // namespace

occlusion_visitor::occlusion_visitor(const vector3d &a1, const vector3d &a3, Float a4, region *)
    : field_4(a1), field_10(a3), field_1C(a4)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(&occlusion_table);
    const double length_squared =
        double(field_10.x) * field_10.x + double(field_10.y) * field_10.y + double(field_10.z) * field_10.z;
    if (length_squared > 9.999999439624929e-11) {
        const double inverse_length = 1.0 / std::sqrt(length_squared);
        field_10.x = field_10.x * inverse_length;
        field_10.y = field_10.y * inverse_length;
        field_10.z = field_10.z * inverse_length;
    }
}

int occlusion_visitor::visit(const subdivision_node &node)
{
    auto &box = const_cast<subdivision_node_obb_base &>(static_cast<const subdivision_node_obb_base &>(node));
    if (box.visited == subdivision_node_obb_base::visit_key()) {
        return 0;
    }
    box.visited = subdivision_node_obb_base::visit_key();
    if ((box.flags & 0x109) != 0) {
        return 0;
    }

    vector4d half4{}, row_x4{}, row_y4{}, row_z4{};
    const bool oriented = box.unpack_xform(half4, row_x4, row_y4, row_z4);
    const vector3d original_half{half4};
    if (scalar_product(original_half, original_half) < 20.0f) {
        return 0;
    }

    const float forward_distance = scalar_product(field_10, box.center - field_4);
    vector3d half = original_half;
    vector3d direction = field_10;
    const vector3d row_x{row_x4}, row_y{row_y4}, row_z{row_z4};
    bool swap_xy = false;
    bool swap_yz = false;
    if (oriented) {
        direction = row_x * field_10.x + row_y * field_10.y + row_z * field_10.z;
        const vector3d vertical = component_product(original_half, row_y);
        const float vx = std::fabs(vertical.x);
        const float vy = std::fabs(vertical.y);
        const float vz = std::fabs(vertical.z);
        if (vx > vz) {
            if (vx > vy) {
                std::swap(half.x, half.y);
                std::swap(direction.x, direction.y);
                swap_xy = true;
            }
        } else if (vz > vy) {
            std::swap(half.y, half.z);
            std::swap(direction.y, direction.z);
            swap_yz = true;
        }
    }

    const vector3d absolute_direction{std::fabs(direction.x), std::fabs(direction.y), std::fabs(direction.z)};
    const float area_first = std::fabs(double(half.z) * direction.x - double(direction.z) * half.x) * half.y;
    const float area_second = std::fabs(double(half.z) * direction.x + double(direction.z) * half.x) * half.y;
    const float area_top = double(absolute_direction.y) * (double(half.z) * half.x);
    const float projected_x = double(half.x) * direction.x;
    const float projected_y = double(half.y) * direction.y;
    const vector3d projected = component_product(half, absolute_direction);
    const double base_distance = double(projected.y) + forward_distance;
    const auto qualifies = [](float area, double distance) {
        return area >= 16.0f && distance > float(LARGE_EPSILON) && area >= distance * distance * 0.01875000074505806f;
    };
    int face_mask = 0;
    if (qualifies(area_first, std::fabs(double(projected_y) + projected_x) + base_distance)) {
        face_mask = 1;
    }
    if (qualifies(area_second, base_distance + std::fabs(double(projected_x) - projected_y))) {
        face_mask |= 2;
    }

    const double top_distance = double(half.x) * absolute_direction.x + double(half.z) * absolute_direction.z +
                                double(half.y) * direction.y + forward_distance;
    if (qualifies(area_top, top_distance)) {
        face_mask |= 3;
    }
    if (face_mask == 0) {
        return 0;
    }

    vector3d back_top, front_top, right_top, left_top;
    vector3d back_bottom, front_bottom, right_bottom, left_bottom;
    if (oriented) {
        const auto scaled_x = component_product(row_x, original_half);
        const auto scaled_y = component_product(row_y, original_half);
        const auto scaled_z = component_product(row_z, original_half);
        const vector3d columns[3] = {{scaled_x.x, scaled_y.x, scaled_z.x},
                                     {scaled_x.y, scaled_y.y, scaled_z.y},
                                     {scaled_x.z, scaled_y.z, scaled_z.z}};
        const vector3d horizontal_x = columns[swap_xy ? 1 : 0];
        const vector3d horizontal_z = columns[swap_yz ? 1 : 2];
        vector3d vertical = columns[swap_xy ? 0 : (swap_yz ? 2 : 1)];
        if (vertical.y < 0.0f) {
            vertical = -vertical;
        }
        vector3d corner = box.center + horizontal_x + vertical;
        front_top = corner + horizontal_z;
        right_top = corner - horizontal_z;
        corner -= vertical * 2.0f;
        front_bottom = corner + horizontal_z;
        right_bottom = corner - horizontal_z;
        corner -= horizontal_x * 2.0f;
        left_bottom = corner + horizontal_z;
        back_bottom = corner - horizontal_z;
        corner += vertical * 2.0f;
        left_top = corner + horizontal_z;
        back_top = corner - horizontal_z;
        if (field_1C - float(LARGE_EPSILON) < field_4.y) {
            const float length = std::sqrt(double(vertical.x) * vertical.x + double(vertical.y) * vertical.y +
                                           double(vertical.z) * vertical.z);
            if (length > float(LARGE_EPSILON)) {
                const vector3d unit_vertical = vertical / length;
                const float ground = field_1C + float(LARGE_EPSILON);
                if (std::fabs(unit_vertical.y) > 0.85000002f && (left_bottom.y < ground || front_bottom.y < ground ||
                                                                 right_bottom.y < ground || back_bottom.y < ground)) {
                    const vector3d extension{0.0f, -100.0f, 0.0f};
                    left_bottom += extension;
                    front_bottom += extension;
                    right_bottom += extension;
                    back_bottom += extension;
                }
            }
        }
    } else {
        vector3d bottom_center = box.center;
        if (field_1C - float(LARGE_EPSILON) < field_4.y && box.center.y - half.y < field_1C + float(LARGE_EPSILON)) {
            bottom_center += vector3d{0.0f, -100.0f, 0.0f};
        }
        back_top = box.center - component_product(half, {1.0f, -1.0f, 1.0f});
        right_top = box.center + component_product(half, {1.0f, 1.0f, -1.0f});
        front_top = box.center + half;
        left_top = box.center + component_product(half, {-1.0f, 1.0f, 1.0f});
        left_bottom = bottom_center - component_product(half, {1.0f, 1.0f, -1.0f});
        front_bottom = bottom_center + component_product(half, {1.0f, -1.0f, 1.0f});
        right_bottom = bottom_center - component_product(half, {-1.0f, 1.0f, 1.0f});
        back_bottom = bottom_center - half;
    }


    if ((face_mask & 1) != 0) {
        occlusion::add_quad_to_database({{back_top, front_top, front_bottom, back_bottom}});
    }
    if ((face_mask & 2) != 0) {
        occlusion::add_quad_to_database({{right_top, left_top, left_bottom, right_bottom}});
    }
    if ((face_mask & 3) != 0) {
        occlusion::add_quad_to_database({{back_top, right_top, front_top, left_top}});
    }
    return 0;
}
