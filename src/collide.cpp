#include "collide.h"
#include "collide_trajectories.h"
#include "collide_aux.h"

#include "collision_capsule.h"
#include "colmesh.h"
#include "colmesh_common.h"
#include "custom_math.h"
#include "entity.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "local_collision.h"
#include "intraframe_trajectory.h"
#include "line_segment.h"
#include "log.h"
#include "mesh_triangle_intersection_record.h"
#include "ngl_math.h"
#include "oldmath_po.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "vector3d.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <utility>

bool find_sphere_intersection(const vector3d &a1, Float a2, const local_collision::entfilter_base &a3,
                              const local_collision::obbfilter_base &a4, vector3d *a5, vector3d *a6, entity **a7,
                              subdivision_node_obb_base **a8)
{
    TRACE("find_sphere_intersection");

    if constexpr (1) {
        local_collision::intersection_list_t best_isect{};

        local_collision::query_args_t v15{};

        auto *v8 = local_collision::query_sphere(a1, a2, a3, a4, v15);
        bool result = local_collision::get_closest_sphere_intersection(v8, a1, a2, a5, a6, &best_isect);

        local_collision::primitive_list_t *v12 = nullptr;
        for (auto *it = v8; it != nullptr; it = v12) {
            v12 = it->field_0;
            local_collision::primitive_list_t::pool.remove(it);
        }

        if (result && best_isect.is_ent) {
            if (a7 != nullptr) {
                *a7 = static_cast<entity *>(best_isect.intersection_node);
            }
        } else if (result) {
            assert(static_cast<subdivision_node_obb_base *>(best_isect.intersection_node)->is_obb_node());

            if (a8 != nullptr) {
                *a8 = static_cast<subdivision_node_obb_base *>(best_isect.intersection_node);
            }
        }

        return result;
    } else {
        bool (*func)(const vector3d *a1,
                     Float a2,
                     const local_collision::entfilter_base *a3,
                     const local_collision::obbfilter_base *a4,
                     vector3d *,
                     vector3d *,
                     entity **,
                     subdivision_node_obb_base **) = CAST(func, 0x005B9F30);
        return func(&a1, a2, &a3, &a4, a5, a6, a7, a8);
    }
}

namespace {

bool sphere_mesh_box_overlap(const vector3d &center, const collision_obb_t &box, float radius_squared)
{
    const vector3d relative = center - box.field_0;
    const vector3d *axes[3]{&box.field_10, &box.axis_y, &box.axis_z};
    float distance_squared = 0.0f;
    for (const auto *axis : axes) {
        const float extent = std::sqrt(axis->length2());
        const float coordinate = std::fabs(dot(*axis, relative) / extent);
        const float distance = coordinate - std::min(extent, coordinate);
        distance_squared += distance * distance;
    }
    return distance_squared <= radius_squared;
}


vector3d sphere_triangle_point(const vector3d &center, const vector3d &a, const vector3d &b, const vector3d &c,
                               const vector3d &ab, const vector3d &bc, const vector3d &normal)
{
    const vector3d ca = a - c;
    const vector3d relative = center - b;
    if (dot(relative, vector3d::cross(normal, ab)) < 0.0f) {
        const float t = -dot(ab, relative);
        if (t >= 0.0f) {
            if (t <= ab.length2())
                return b - (t / ab.length2()) * ab;
            const float u = dot(ca, center - c);
            if (u <= 0.0f)
                return c;
            if (u >= ca.length2())
                return a;
            return c + (u / ca.length2()) * ca;
        }
        const float u = dot(bc, relative);
        if (u <= 0.0f)
            return b;
        if (u >= bc.length2())
            return c;
        return b + (u / bc.length2()) * bc;
    }
    if (dot(relative, vector3d::cross(normal, bc)) < 0.0f) {
        const float t = dot(bc, relative);
        if (t <= 0.0f)
            return b;
        if (t < bc.length2())
            return b + (t / bc.length2()) * bc;
        const float u = dot(ca, center - c);
        if (u <= 0.0f)
            return c;
        if (u >= ca.length2())
            return a;
        return c + (u / ca.length2()) * ca;
    }
    const float t = dot(ca, center - c);
    if (dot(center - c, vector3d::cross(normal, ca)) < 0.0f) {
        if (t <= 0.0f)
            return c;
        if (t >= ca.length2())
            return a;
        return c + (t / ca.length2()) * ca;
    }
    return center - (dot(normal, relative) / normal.length2()) * normal;
}
}  // namespace

bool best_sphere_obb_tree_intersection(math::VecClass<3, 0> const &a1, const cg_mesh *mesh, const collision_obb_t &node,
                                       math::VecClass<3, 0> &nearest, float &radius_squared)
{
    if constexpr (!STANDALONE_SYSTEM)
        return CDECL_CALL(0x005CA780, &a1, mesh, &node, &nearest, &radius_squared);
    const vector3d center{a1.x, a1.y, a1.z};
    bool hit = false;
    if (!node.field_34) {
        for (uint8_t child = 0; child < node.field_35; ++child) {
            const auto &box = mesh->data->field_10[node.field_36 + child];
            if (sphere_mesh_box_overlap(center, box, radius_squared))
                hit |= best_sphere_obb_tree_intersection(a1, mesh, box, nearest, radius_squared);
        }
        return hit;
    }
    const auto &list = node.triangles();
    const auto *indices = list.triangles();
    constexpr float epsilon = 0.00009999999747378752f;
    for (uint16_t triangle = 0; triangle < list.triangle_count; ++triangle) {
        const auto &a = list.vertices[indices[triangle].vertex[0]];
        const auto &b = list.vertices[indices[triangle].vertex[1]];
        const auto &c = list.vertices[indices[triangle].vertex[2]];
        const vector3d ab = b - a;
        const vector3d bc = c - b;
        const vector3d normal = vector3d::cross(ab, bc);
        const float normal_squared = normal.length2();
        if (normal_squared < epsilon)
            continue;
        const float plane = dot(normal, center - b);
        if (plane < epsilon || normal_squared * radius_squared <= plane * plane)
            continue;
        const vector3d point = sphere_triangle_point(center, a, b, c, ab, bc, normal);
        const float distance_squared = (center - point).length2();
        if (distance_squared < radius_squared) {
            radius_squared = distance_squared;
            nearest.x = point.x;
            nearest.y = point.y;
            nearest.z = point.z;
            hit = true;
        }
    }
    return hit;
}

bool collide_sphere_mesh(const vector3d &center, Float radius, const cg_mesh *mesh, vector3d *hit_loc,
                         vector3d *hit_norm)
{
    if constexpr (!STANDALONE_SYSTEM)
        return CDECL_CALL(0x005CB2D0, &center, radius, mesh, hit_loc, hit_norm);
    float radius_squared = radius * radius;
    if (!sphere_mesh_box_overlap(center, mesh->data->field_10[0], radius_squared))
        return false;
    math::VecClass<3, 0> input{}, nearest{};
    input.x = center.x;
    input.y = center.y;
    input.z = center.z;
    if (!best_sphere_obb_tree_intersection(input, mesh, mesh->data->field_10[0], nearest, radius_squared))
        return false;
    *hit_loc = vector3d{nearest.x, nearest.y, nearest.z};
    const vector3d relative = center - *hit_loc;
    *hit_norm = relative / std::sqrt(std::fabs(relative.length2()));
    return true;
}

bool collide_sphere_geometry(const vector3d &a2, Float a3, collision_geometry *cg, const po &a4, vector3d *impact_pos,
                             vector3d *impact_normal)
{
    if (cg->get_type() == collision_geometry::CAPSULE) {
        auto a2a = bit_cast<collision_capsule *>(cg)->get_abs_capsule(a4);

        float v23;
        closest_point_line_segment_point(a2a.base, a2a.end, a2, v23);
        auto a1 = sub_48B5B0(a2a.base, a2a.end, v23);

        auto radius = a2a.radius + a3;

        if (radius * radius > (a1 - a2).length2()) {
            *impact_normal = a2 - a1;
            impact_normal->normalize();

            *impact_pos = a2a.radius * (*impact_normal) + a1;

            return true;
        }
    } else if (cg->get_type() == collision_geometry::MESH) {
        auto v25 = *a4.inverse();
        auto v22 = v25.slow_xform(a2);
        if (collide_sphere_mesh(v22, a3, bit_cast<cg_mesh *>(cg), impact_pos, impact_normal)) {
            *impact_pos = a4.m * *impact_pos;
            *impact_normal = sub_55DCB0(a4.m, *impact_normal);
            return true;
        }
    }

    return false;
}

bool collide_sphere_entity(const vector3d &a1, Float a2, const entity *ent, vector3d *a4, vector3d *a5, po *a6)
{
    auto *cg = ent->get_colgeom();

    assert(ent->is_an_actor());

    assert("Entity without collision geometry passed to collide_sphere_entity." && cg != nullptr);

    if (cg == nullptr) {
        return false;
    }

    auto v18 = ent->get_colgeom_radius();
    auto *v11 = a6 ? a6 : &bit_cast<entity *>(ent)->get_abs_po();
    auto *v17 = v11;
    auto v10 = cg->get_local_space_bounding_sphere_center();
    auto &v6 = v17->get_matrix();
    auto v16 = v6 * v10;
    auto v7 = a1 - v16;
    auto v15 = v7.length2();
    auto v8 = (v18 + a2) * (v18 + a2);
    if (v8 + 0.000099999997 < v15) {
        return false;
    }

    return collide_sphere_geometry(a1, a2, cg, *v17, a4, a5);
}

bool collide_segment_solid_sphere(const vector3d &a1, const vector3d &a2, const vector3d &a3, Float radius,
                                  vector3d *hit_loc)
{
    if constexpr (1) {
        auto local_vec0 = a2 - a1;
        auto local_vec1 = a1 - a3;

        auto length2_0 = local_vec0.length2();
        if (length2_0 < 0.000001f) {
            if (radius * radius > local_vec1.length2()) {
                *hit_loc = a1;
                return true;
            }

            return false;
        }

        auto local_dot = dot(local_vec0, local_vec1);
        auto a2a = local_dot + local_dot;
        auto v16 = a2a * a2a - (local_vec1.length2() - radius * radius) * length2_0 * 4.0f;
        if (v16 < 0.0f) {
            return false;
        }

        auto v17 = -a2a;
        auto v18 = std::sqrt(v16);
        auto v19 = 1.0f / (2.0 * length2_0);
        auto a2b = (v17 - v18) * v19;
        auto a1b = (v18 + v17) * v19;

        static constexpr float flt_87E6EC = -EPSILON;
        static constexpr float flt_8912D8 = 1.0001;

        if (a1b < flt_87E6EC || a2b > flt_8912D8) {
            return false;
        }

        if (a2b < flt_87E6EC) {
            if (a1b > flt_8912D8) {
                *hit_loc = a1;
            } else {
                auto v21 = local_vec0 * a1b;
                *hit_loc = a1 + v21;
            }

        } else {
            auto v20 = local_vec0 * a2b;
            *hit_loc = a1 + v20;
        }

        return true;
    } else {
        return (bool)CDECL_CALL(0x005B9430, &a1, &a2, &a3, radius, hit_loc);
    }
}

int collide_segment_hollow_sphere(const vector3d &a1, const vector3d &a2, const vector3d &a3, Float a4, vector3d *a5)
{
    if constexpr (1) {
        auto local_vec0 = a2 - a1;
        auto local_vec1 = a1 - a3;

        int result;
        auto length2 = local_vec0.length2();
        if (length2 >= 0.000001f) {
            int v14 = 0;
            auto v15 = dot(local_vec0, local_vec1);
            auto v16 = v15 + v15;
            auto v17 = v16 * v16 - (local_vec1.length2() - a4 * a4) * length2 * 4.0f;
            if (v17 >= 0.0f) {
                auto v18 = v16 * -1.f;
                auto a3a = v17;
                auto v19 = sqrt(a3a);
                auto v20 = 1.f / (2.0 * length2);
                auto v30 = (v18 - v19) * v20;
                auto v28 = (v19 + v18) * v20;

                static constexpr float flt_87E6EC = -EPSILON;
                static constexpr float flt_8912D8 = 1.0001;

                if (v30 >= flt_87E6EC && v30 <= flt_8912D8) {
                    a5[v14++] = local_vec0 * v30 + a1;
                }

                if (v28 >= flt_87E6EC && v28 <= flt_8912D8 && !approx_equals(v30, v28, EPSILON)) {
                    auto v23 = local_vec0 * v28;
                    a5[v14++] = a1 + v23;
                }
            }

            result = v14;
        } else if (equal<float>(a4 * a4, local_vec1.length2())) {
            *a5 = a1;
            result = 1;
        } else {
            result = 0;
        }

        return result;
    } else {
        return CDECL_CALL(0x005B9660, &a1, &a2, &a3, a4, a5);
    }
}

float dist_point_segment_sq_opt(const vector3d &a1, const vector3d &a2, const vector3d &a3)
{
    auto &v3 = a3;
    auto &v4 = a2;
    auto &v5 = a1;

    float tmp;
    closest_point_line_segment_point(a2, a3, a1, tmp);
    auto v13 = v3[2] - v4[2];
    auto v8 = (v3[0] - v4[0]) * tmp;
    auto v11 = v8 + v4[0];
    auto v12 = (v3[1] - v4[1]) * tmp + v4[1];
    auto v9 = v11 - v5[0];
    auto v10 = v12 - v5[1];
    auto v6 = v13 * tmp + v4[2] - v5[2];
    return v6 * v6 + v10 * v10 + v9 * v9;
}

namespace {


bool segment_cylinder_intersection(const vector3d &start, const vector3d &direction, const vector3d &base,
                                   const vector3d &axis, float radius, float length, vector3d *point)
{
    vector3d local_start = start - base;
    vector3d local_direction = direction;
    vector3d rotation_axis{-axis.z, 0.0f, axis.x};
    const float sine_squared = rotation_axis.length2();
    if (sine_squared >= 9.99999993922529e-9f) {
        const float sine = std::sqrt(sine_squared);
        rotation_axis *= 1.0f / sine;
        const auto rotate = [&](const vector3d &v) {
            return vector3d::cross(rotation_axis, v) * sine + v * axis.y +
                   rotation_axis * (dot(rotation_axis, v) * (1.0f - axis.y));
        };
        local_start = rotate(local_start);
        local_direction = rotate(local_direction);
    }
    const float radius_squared = radius * radius;
    float upper = -1.0f, lower = -1.0f;


    float fraction = 0.0f;
    if (!(local_direction.y <= 0.0f && local_direction.y >= 0.0f)) {
        const float reciprocal = 1.0f / local_direction.y;
        const auto cap = [&](float height) {
            fraction = (height - local_start.y) * reciprocal;
            if (fraction < 0.0f || fraction > 1.0f)
                return -1.0f;
            const float x = local_start.x + local_direction.x * fraction;
            const float z = local_start.z + local_direction.z * fraction;
            return x * x + z * z <= radius_squared ? fraction : -1.0f;
        };
        upper = cap(length);
        lower = cap(-length);
    }
    const float quadratic = local_direction.x * local_direction.x + local_direction.z * local_direction.z;
    const float linear = 2.0f * (local_direction.x * local_start.x + local_direction.z * local_start.z);
    const float discriminant =
        linear * linear -
        (local_start.x * local_start.x + local_start.z * local_start.z - radius_squared) * quadratic * 4.0f;
    if (discriminant >= 0.0f && !(quadratic <= 0.0f && quadratic >= 0.0f)) {
        fraction = -1.0f;
        const float root = std::sqrt(discriminant);
        const float reciprocal = 1.0f / (quadratic + quadratic);
        const float first = (root - linear) * reciprocal;
        const float second = (-linear - root) * reciprocal;
        if (first >= 0.0f && first <= 1.0f)
            fraction = first;
        if (second >= 0.0f && second <= 1.0f && (fraction < 0.0f || second < fraction))
            fraction = second;
    }
    if (fraction >= 0.0f && std::abs(local_start.y + local_direction.y * fraction) > length)
        fraction = -1.0f;
    if (upper >= 0.0f && (fraction < 0.0f || upper < fraction))
        fraction = upper;
    if (lower >= 0.0f && (fraction < 0.0f || lower < fraction))
        fraction = lower;
    if (fraction < 0.0f)
        return false;
    *point = start + direction * fraction;
    return true;
}
}  // namespace

bool collide_segment_capsule(const vector3d &start, const vector3d &end, const vector3d &base, const vector3d &tip,
                             Float radius, vector3d *hit_point, vector3d *hit_normal)
{
    const vector3d capsule_direction = tip - base;
    const float capsule_length = capsule_direction.length();
    const vector3d axis = capsule_length >= EPSILON ? capsule_direction / capsule_length : YVEC;
    vector3d candidates[6];
    int count =
        segment_cylinder_intersection(start, end - start, base, axis, radius.value, capsule_length, candidates) ? 1 : 0;
    count += collide_segment_hollow_sphere(start, end, base, radius, candidates + count);
    count += collide_segment_hollow_sphere(start, end, tip, radius, candidates + count);
    bool hit = false;
    for (int i = 0; i < count; ++i) {
        float projection = closest_point_infinite_line_point(base, capsule_direction, candidates[i]);
        projection = projection >= 0.0f ? (projection > 1.0f ? 1.0f : projection) : 0.0f;
        const vector3d nearest = base + capsule_direction * projection;
        const float distance_squared = (nearest - candidates[i]).length2();


        if (std::abs(distance_squared - radius.value * radius.value) < 0.009999999776482582f &&
            (start - candidates[i]).length2() < 3.402823466e38f) {
            *hit_point = candidates[i];
            *hit_normal = candidates[i] - nearest;
            hit = true;
        }
    }
    if (hit) {
        const float squared = hit_normal->length2();
        if (squared > 9.999999439624929e-11f)
            *hit_normal *= 1.0f / std::sqrt(squared);
    }
    return hit;
}

namespace {


bool segment_triangle_intersection(const vector3d &start, const vector3d &end, const vector3d &a, const vector3d &b,
                                   const vector3d &c, vector3d &point)
{
    constexpr float epsilon = 0.00009999999747378752f;
    const vector3d edge_b = b - a;
    const vector3d edge_c = c - a;
    const vector3d normal = vector3d::cross(edge_b, edge_c);
    if (normal.length2() < epsilon)
        return false;
    const vector3d direction = end - start;
    const float denominator = dot(direction, normal);
    if (std::abs(denominator) < epsilon)
        return false;
    const float t = dot(a - start, normal) / denominator;
    if (t < 0.0f || t > 1.0f)
        return false;
    point = start + direction * t;
    int u = 0, v = 1;
    if (std::abs(normal.x) > std::abs(normal.y) && std::abs(normal.x) > std::abs(normal.z)) {
        u = 1;
        v = 2;
    } else if (std::abs(normal.y) > std::abs(normal.z)) {
        v = 2;
    }
    if (std::abs(edge_b[u]) < epsilon)
        std::swap(u, v);
    if (std::abs(edge_b[u]) < epsilon)
        return false;
    const vector3d relative = point - a;
    const float beta =
        (relative[u] * edge_b[v] - relative[v] * edge_b[u]) / (edge_c[u] * edge_b[v] - edge_c[v] * edge_b[u]);
    constexpr float lower = -0.000009999999747378752f;
    constexpr float upper = 1.0000100135803223f;
    if (beta < lower || beta > upper)
        return false;
    const float alpha = (relative[u] - edge_c[u] * beta) / edge_b[u];
    return alpha >= lower && alpha <= upper && alpha + beta <= upper;
}

}  // namespace

bool get_closest_intersection_from_list(const vector3d &a3, const cg_mesh *mesh,
                                        mesh_triangle_intersection_record_t *a2, vector3d *closest_point,
                                        vector3d *closest_normal)
{
    mesh_triangle_intersection_record_t *v5 = nullptr;
    float v19 = 3.4028235e38;
    if (a2 == nullptr) {
        return false;
    }

    do {
        auto v8 = a2->field_0[0] - a3[0];
        auto v6 = a2->field_0[1] - a3[1];
        auto v7 = a2->field_0[2] - a3[2];
        auto v9 = v7 * v7 + v6 * v6 + v8 * v8;
        if (v9 < v19) {
            v19 = v9;
            v5 = a2;
        }

        a2 = a2->field_14;
    } while (a2);

    if (v5 == nullptr) {
        return false;
    }

    assert(closest_normal != nullptr && closest_point != nullptr);

    *closest_point = v5->field_0;
    const auto &triangles = mesh->data->field_10[v5->field_10].triangles();
    const auto &indices = triangles.triangles()[v5->field_C];
    const vector3d &a = triangles.vertices[indices.vertex[0]];
    const vector3d &b = triangles.vertices[indices.vertex[1]];
    const vector3d &c = triangles.vertices[indices.vertex[2]];
    *closest_normal = vector3d::cross(b - a, c - a);

    auto v16 = closest_normal->length2();

    if (v16 > 9.9999994e-11) {
        auto v18 = 1.f / sqrt(v16);
        *closest_normal *= v18;
    }

    return true;
}

void line_segment_obb_tree_intersection(const vector3d &start, const vector3d &end, const cg_mesh *mesh,
                                        const collision_obb_t *node, mesh_triangle_intersection_record_t **hits)
{
    if (node->field_34) {
        const auto &list = node->triangles();
        int closest_triangle = -1;
        float closest_distance = 3.402823466e38f;
        vector3d closest_point;
        for (int triangle = 0; triangle < list.triangle_count; ++triangle) {
            const auto &indices = list.triangles()[triangle];
            vector3d point;
            if (segment_triangle_intersection(start,
                                              end,
                                              list.vertices[indices.vertex[0]],
                                              list.vertices[indices.vertex[1]],
                                              list.vertices[indices.vertex[2]],
                                              point)) {
                const float distance = (point - start).length2();
                if (distance < closest_distance) {
                    closest_distance = distance;
                    closest_triangle = triangle;
                    closest_point = point;
                }
            }
        }
        if (closest_triangle < 0)
            return;
        auto &pool = mesh_triangle_intersection_record_t::pool();


        if (!pool.m_initialized)
            pool.init(sizeof(mesh_triangle_intersection_record_t), 32, 4, 1, 0, nullptr);
        auto *hit = static_cast<mesh_triangle_intersection_record_t *>(pool.allocate_new_block());
        hit->field_0 = closest_point;
        hit->field_C = closest_triangle;
        hit->field_10 = static_cast<int>(node - mesh->data->field_10);
        hit->field_14 = *hits;
        *hits = hit;
    } else if (segment_mesh_box_overlap(start, end, *node)) {
        for (int child = 0; child < node->field_35; ++child)
            line_segment_obb_tree_intersection(start, end, mesh, &mesh->data->field_10[node->field_36 + child], hits);
    }
}

bool collide_segment_mesh(const vector3d &a3, const vector3d &a2, cg_mesh *mesh, vector3d *closest_point,
                          vector3d *closest_normal)
{
    assert(mesh != nullptr);

    auto *v11 = mesh->data->field_10;
    mesh_triangle_intersection_record_t *v12 = nullptr;
    line_segment_obb_tree_intersection(a3, a2, mesh, v11, &v12);
    auto result = get_closest_intersection_from_list(a3, mesh, v12, closest_point, closest_normal);

    while (v12 != nullptr) {
        auto *next = v12->field_14;
        mesh_triangle_intersection_record_t::pool().remove(v12);
        v12 = next;
    }

    return result;
}

bool collide_segment_geometry(const vector3d &a2, const vector3d &a3, collision_geometry *cg, const po &a5,
                              vector3d *impact_pos, vector3d *impact_normal)
{
    assert(impact_pos != nullptr && impact_normal != nullptr);

    bool result;
    if (cg->get_type() == collision_geometry::CAPSULE) {
        auto v11 = bit_cast<collision_capsule *>(cg)->get_abs_capsule(a5);
        result = collide_segment_capsule(a2, a3, v11.base, v11.end, v11.radius, impact_pos, impact_normal);
    } else if (cg->get_type() == collision_geometry::MESH) {
        auto v12 = *a5.inverse();
        auto local_p0 = v12.slow_xform(a2);

        assert(local_p0.is_valid());

        auto local_p1 = v12.slow_xform(a3);

        assert(local_p1.is_valid());

        result = collide_segment_mesh(local_p0, local_p1, bit_cast<cg_mesh *>(cg), impact_pos, impact_normal);
        auto v7 = result;
        if (result) {
            assert(impact_pos->is_valid() && "collide_segment_geometry failed");

            *impact_pos = a5.m * *impact_pos;

            assert(impact_pos->is_valid() && "collide_segment_geometry transform failed");

            assert(impact_normal->is_valid() && "collide_segment_geometry failed");

            *impact_normal = sub_55DCB0(a5.m, *impact_normal);

            assert(impact_normal->is_valid() && "collide_segment_geometry transform failed");

            result = v7;
        }
    } else {
        assert("Invalid collision geometry type -- please report to Andrei." && 0);
        result = false;
    }

    return result;
}

bool collide_segment_entity(const vector3d &start, const vector3d &end, const entity *ent, const po &transform,
                            vector3d *point, vector3d *normal)
{
    auto *geometry = ent->get_colgeom();
    const float radius = ent->get_colgeom_radius();
    const vector3d center = transform.m * geometry->get_local_space_bounding_sphere_center();
    return radius * radius >= dist_point_segment_sq_opt(center, start, end) &&
           collide_segment_geometry(start, end, geometry, transform, point, normal);
}

bool collide_segment_entity_or_sphere(const vector3d &a2, const vector3d &a3, const entity *a4, const po &a5,
                                      vector3d *a6, vector3d *a7, Float a8)
{
    if (a4->colgeom != nullptr) {
        return collide_segment_entity(a2, a3, a4, a5, a6, a7);
    }

    vector3d hit_loc{};
    if (!collide_segment_solid_sphere(a2, a3, a5.get_position(), a8, &hit_loc)) {
        return false;
    }

    *a6 = hit_loc;

    *a7 = a2 - hit_loc;
    a7->normalize();

    return true;
}

bool collide_capsule_capsule(const vector3d &a1, const vector3d &a2, Float radius1, const vector3d &a4,
                             const vector3d &a5, Float radius2, vector3d &cp1, vector3d &cp2, vector3d &normal)
{
    float time_a, time_b;
    closest_point_line_segment_line_segment(a1, a2, a4, a5, &time_a, &time_b);
    const vector3d axis_a = a1 + (a2 - a1) * time_a;
    const vector3d axis_b = a4 + (a5 - a4) * time_b;
    const float distance = (axis_b - axis_a).length();
    if (radius1 + radius2 < distance)
        return false;
    if (distance <= 0.000099999997f) {
        const vector3d axis = a1 - a2;
        const float x = std::abs(axis.x), y = std::abs(axis.y), z = std::abs(axis.z);
        const int major = x > y && x > z ? 0 : y > z && y > x ? 1 : 2;
        normal = vector3d::cross(axis, major ? vector3d{1.0f, 0.0f, 0.0f} : vector3d{0.0f, 1.0f, 0.0f});
        normal.normalize();
    } else {
        normal = (axis_b - axis_a) / distance;
    }
    cp1 = axis_a + normal * radius1;
    cp2 = axis_b - normal * radius2;
    return true;
}

namespace {


void capsule_triangle_edge_points(const vector3d &start, const vector3d &end, const vector3d &edge_start,
                                  const vector3d &edge_end, vector3d &axis_point, vector3d &triangle_point)
{
    const vector3d axis = end - start;
    const vector3d edge = edge_end - edge_start;
    const vector3d relative = start - edge_start;
    const float a = axis.length2(), b = dot(axis, edge), c = edge.length2();
    const float d = dot(axis, relative), e = dot(edge, relative);
    const float determinant = std::abs(a * c - b * b);
    float axis_time = 0.0f, edge_time = 0.0f;
    float best = 3.402823466e38f;
    auto consider = [&](float s, float t) {
        const float distance = (relative + axis * s - edge * t).length2();
        if (distance < best) {
            best = distance;
            axis_time = s;
            edge_time = t;
        }
    };
    if (determinant >= 0.000099999997f) {
        const float s = (b * e - c * d) / determinant;
        const float t = (a * e - b * d) / determinant;
        if (s >= 0.0f && s <= 1.0f && t >= 0.0f && t <= 1.0f) {
            axis_point = start + axis * s;
            triangle_point = edge_start + edge * t;
            return;
        }
    } else {
        if (b >= 0.0f) {
            if (-d >= a)
                axis_time = 1.0f;
            else if (d <= 0.0f)
                axis_time = -d / a;
            else
                edge_time = d >= b ? 1.0f : d / b;
        } else {
            if (d < 0.0f) {
                if (-d <= a)
                    axis_time = -d / a;
                else {
                    axis_time = 1.0f;
                    edge_time = -(d + a) >= -b ? 1.0f : (d + a) / b;
                }
            }
        }
        axis_point = start + axis * axis_time;
        triangle_point = edge_start + edge * edge_time;
        return;
    }
    consider(0.0f, c > 0.0f ? std::clamp(e / c, 0.0f, 1.0f) : 0.0f);
    consider(1.0f, c > 0.0f ? std::clamp((e + b) / c, 0.0f, 1.0f) : 0.0f);
    consider(a > 0.0f ? std::clamp(-d / a, 0.0f, 1.0f) : 0.0f, 0.0f);
    consider(a > 0.0f ? std::clamp((b - d) / a, 0.0f, 1.0f) : 0.0f, 1.0f);
    axis_point = start + axis * axis_time;
    triangle_point = edge_start + edge * edge_time;
}


bool closest_points_capsule_triangle(const capsule &query, const vector3d &a, const vector3d &b, const vector3d &c,
                                     vector3d &axis_point, vector3d &triangle_point, vector3d &normal)
{
    constexpr float epsilon = 0.000099999997f;
    const vector3d vertices[3]{a, b, c};
    const vector3d edges[3]{b - a, c - b, a - c};
    normal = vector3d::cross(edges[0], edges[1]);
    const float normal_squared = normal.length2();
    if (normal_squared < epsilon)
        return false;
    const float start_plane = dot(query.base - b, normal);
    const float end_plane = dot(query.end - b, normal);
    const float lowest_plane = std::min(start_plane, end_plane);
    const float highest_plane = std::max(start_plane, end_plane);
    const float radius_squared = query.radius * query.radius;
    if (highest_plane < epsilon ||
        (lowest_plane > 0.0f && lowest_plane * lowest_plane >= radius_squared * normal_squared))
        return false;
    const vector3d high = start_plane >= end_plane ? query.base : query.end;
    const vector3d low = start_plane >= end_plane ? query.end : query.base;
    const vector3d delta = query.end - query.base;

    auto edge_contact = [&](const vector3d &vertex, const vector3d &edge) {
        capsule_triangle_edge_points(query.base, query.end, vertex, vertex + edge, axis_point, triangle_point);
        const vector3d relative = axis_point - triangle_point;
        return dot(relative, normal) >= epsilon && relative.length2() < radius_squared;
    };
    auto vertex_contact = [&](const vector3d &vertex, const vector3d &first, const vector3d &second, int mode) {
        const vector3d start_relative = query.base - vertex;
        const vector3d end_relative = query.end - vertex;
        if (mode == 0) {
            const float from = dot(second, start_relative);
            const float to = dot(second, end_relative);
            if (from > 0.0f && to > 0.0f)
                return edge_contact(vertex, second);
            if (from <= 0.0f && to <= 0.0f)
                return edge_contact(vertex, first);
            const vector3d intersection = delta * from + start_relative * (from - to);
            return edge_contact(vertex, dot(delta, intersection) > 0.0f ? second : first);
        }
        if (mode == 2) {
            const vector3d relative = low - vertex;
            const float projection = dot(second, relative);
            const vector3d direction = high - low;
            const vector3d test = direction * projection - second * dot(direction, relative);
            return edge_contact(vertex, projection > 0.0f && dot(direction, test) > 0.0f ? second : first);
        }

        const float length_squared = delta.length2();
        const float numerator = -dot(delta, start_relative);
        vector3d relative;
        if (numerator > 0.0f && numerator < length_squared) {
            auto beyond_edge = [&](const vector3d &edge) {
                const float from = dot(edge, start_relative);
                const float to = dot(edge, end_relative);
                return (from > 0.0f || to > 0.0f) &&
                       dot(start_relative, delta) * (from - to) + from * length_squared > 0.0f;
            };
            if (beyond_edge(first))
                return edge_contact(vertex, first);
            if (beyond_edge(second))
                return edge_contact(vertex, second);
            relative = start_relative + delta * (numerator / length_squared);
            if (dot(relative, normal) < epsilon || relative.length2() > radius_squared)
                return false;
            triangle_point = vertex;
            axis_point = vertex + relative;
            return true;
        }
        axis_point = numerator > 0.0f ? query.end : query.base;
        relative = axis_point - vertex;
        float projection = dot(first, relative);
        if (projection > 0.0f)
            relative -= first * (projection / first.length2());
        else {
            projection = dot(second, relative);
            if (projection > 0.0f)
                relative -= second * (projection / second.length2());
        }
        if (dot(relative, normal) < epsilon || relative.length2() >= radius_squared)
            return false;
        triangle_point = axis_point - relative;
        return true;
    };
    auto outside_edge = [&](int edge, int mode) {
        const int previous = (edge + 2) % 3;
        const int next = (edge + 1) % 3;
        if (dot(edges[previous], edges[edge]) > 0.0f)
            return vertex_contact(vertices[edge], edges[edge], -edges[previous], mode);
        if (dot(edges[next], edges[edge]) > 0.0f)
            return vertex_contact(vertices[next], -edges[edge], edges[next], mode);
        return edge_contact(vertices[edge], edges[edge]);
    };

    float lower_numerator = 0.0f, lower_denominator = 1.0f;
    float upper_numerator = 1.0f, upper_denominator = 1.0f;
    int lower_edge = 0, upper_edge = 0;
    bool separated = false;
    for (int edge = 0; edge != 3; ++edge) {
        const vector3d inward = vector3d::cross(normal, edges[edge]);
        const float from = dot(high - vertices[edge], inward);
        const float to = dot(low - vertices[edge], inward);
        if (from < 0.0f && to < 0.0f)
            return outside_edge(edge, 0);
        if (from < 0.0f) {
            const float numerator = -from;
            const float denominator = to - from;
            if (denominator * lower_numerator < numerator * lower_denominator) {
                lower_numerator = numerator;
                lower_denominator = denominator;
                lower_edge = edge;
                separated = numerator * upper_denominator > upper_numerator * denominator;
            }
        } else if (to < 0.0f) {
            const float denominator = from - to;
            if (denominator * upper_numerator > from * upper_denominator) {
                upper_numerator = from;
                upper_denominator = denominator;
                upper_edge = edge;
                separated = denominator * lower_numerator > from * lower_denominator;
            }
        }
        if (separated)
            break;
    }
    if (separated) {
        const int sum = lower_edge + upper_edge;
        const int vertex = sum == 1 ? 1 : sum == 2 ? 0 : 2;
        return vertex_contact(vertices[vertex], edges[vertex], -edges[(vertex + 2) % 3], 1);
    }
    if ((epsilon - highest_plane) * upper_denominator > (lowest_plane - highest_plane) * upper_numerator)
        return false;
    if (upper_numerator <= 1.0f && upper_numerator >= 1.0f && upper_denominator <= 1.0f && upper_denominator >= 1.0f) {
        axis_point = low;
        triangle_point = low - normal * (lowest_plane / normal_squared);
        return true;
    }
    return outside_edge(upper_edge, 2);
}

void capsule_mesh_tree_intersections(const capsule &query, const vector3d &center, float radius_squared,
                                     const cg_mesh *mesh, const collision_obb_t &node,
                                     local_collision::closest_points_pair_t **pairs)
{
    if (!node.field_34) {
        if (!sphere_mesh_box_overlap(center, node, radius_squared))
            return;
        for (uint8_t child = 0; child != node.field_35; ++child)
            capsule_mesh_tree_intersections(
                query, center, radius_squared, mesh, mesh->data->field_10[node.field_36 + child], pairs);
        return;
    }
    const auto &triangles = node.triangles();
    const auto *indices = triangles.triangles();
    for (uint16_t index = 0; index != triangles.triangle_count; ++index) {
        vector3d axis_point, triangle_point, normal;
        if (!closest_points_capsule_triangle(query,
                                             triangles.vertices[indices[index].vertex[0]],
                                             triangles.vertices[indices[index].vertex[1]],
                                             triangles.vertices[indices[index].vertex[2]],
                                             axis_point,
                                             triangle_point,
                                             normal))
            continue;
        normal /= std::sqrt(normal.length2());
        const vector3d relative = axis_point - triangle_point;
        auto *pair = local_collision::allocate_closest_points_pair();
        pair->point = axis_point;
        pair->other_point = triangle_point;
        pair->direction =
            relative.length2() <= 9.99999905104687e-09f ? -normal : relative * (-1.0f / std::sqrt(relative.length2()));
        pair->normal = normal;
        pair->distance_squared = relative.length2();
        pair->next = *pairs;
        *pairs = pair;
    }
}
}  // namespace

local_collision::closest_points_pair_t *collide_capsule_geometry(const capsule &query, collision_geometry *geometry,
                                                                 const po &transform)
{
    if (geometry->get_type() == collision_geometry::CAPSULE) {
        const capsule other = static_cast<collision_capsule *>(geometry)->get_abs_capsule(transform);
        vector3d query_surface, other_surface, direction;
        if (!collide_capsule_capsule(query.base,
                                     query.end,
                                     query.radius,
                                     other.base,
                                     other.end,
                                     other.radius,
                                     query_surface,
                                     other_surface,
                                     direction))
            return nullptr;
        auto *pair = local_collision::allocate_closest_points_pair();
        pair->point = query_surface - direction * query.radius;
        pair->direction = direction;
        pair->other_point = other_surface;
        pair->normal = -direction;
        pair->distance_squared = (pair->point - pair->other_point).length2();
        return pair;
    }
    const po inverse = *transform.inverse();
    const capsule local{inverse.m * query.base, inverse.m * query.end, query.radius};
    const vector3d center = (local.base + local.end) * 0.5f;
    const float radius = (local.base - center).length() + local.radius;
    auto *mesh = static_cast<cg_mesh *>(geometry);
    local_collision::closest_points_pair_t *pairs = nullptr;
    capsule_mesh_tree_intersections(local, center, radius * radius, mesh, mesh->data->field_10[0], &pairs);
    for (auto *pair = pairs; pair; pair = pair->next) {
        pair->point = transform.m * pair->point;
        pair->other_point = transform.m * pair->other_point;
        pair->direction = sub_55DCB0(transform.m, pair->direction);
        pair->normal = sub_55DCB0(transform.m, pair->normal);
        pair->distance_squared = (pair->point - pair->other_point).length2();
    }
    return pairs;
}

local_collision::closest_points_pair_t *collide_capsule_entity(const capsule &query, const entity *ent,
                                                               const po &transform)
{
    auto *geometry = ent->get_colgeom();
    const vector3d center = transform.m * geometry->get_local_space_bounding_sphere_center();
    const float radius = geometry->get_bounding_sphere_radius() + query.radius;
    if (radius * radius < dist_point_segment_sq_opt(center, query.base, query.end))
        return nullptr;
    return collide_capsule_geometry(query, geometry, transform);
}

void collide_unit_test()
{
    sp_log("collide test\n");
    sp_log("testing collide_segment_solid_sphere\n");

    vector3d hit_loc;
    auto result = collide_segment_solid_sphere(
        vector3d{0.0, 0.0, 0.0}, vector3d{0.0, 2.0, 0.0}, vector3d{0.0, 2.0, 0.0}, 1.0, &hit_loc);
    assert(result);
    assert(hit_loc == vector3d(0.0f, 1.0f, 0.0f));

    result = collide_segment_solid_sphere(
        vector3d{0.0, 0.0, 0.0}, vector3d{1.0, 0.0, 0.0}, vector3d{-1.0, 0.0, 0.0}, 1.0, &hit_loc);

    assert(result);
    assert(hit_loc == vector3d(0.0f, 0.0f, 0.0f));

    result = collide_segment_solid_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 0.0, &hit_loc);

    assert(result);
    assert(hit_loc == vector3d(0.0f, 0.0f, 0.0f));

    result = collide_segment_solid_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(-1.0, 0.0, 0.0), vector3d(-2.0, 0.0, 0.0), 1.0, &hit_loc);
    assert(result);
    assert(hit_loc == vector3d(-1.0f, 0.0f, 0.0f));

    result = collide_segment_solid_sphere(
        vector3d(-1.0, 1.0, 0.0), vector3d(1.0, 1.0, 0.0), vector3d(1.0, 1.0, 0.0), 1.0, &hit_loc);
    assert(result);
    assert(hit_loc == vector3d(0.0f, 1.0f, 0.0f));

    result = collide_segment_solid_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(2.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 1.0, &hit_loc);
    assert(result);

    assert((hit_loc == vector3d(-1.0f, 0.0f, 0.0f)) || (hit_loc == vector3d(1.0f, 0.0f, 0.0f)));

    result = collide_segment_solid_sphere(
        vector3d{-2.0, 2.0, 0.0}, vector3d{2.0, 2.0, 0.0}, vector3d{0.0, 0.0, 0.0}, 1.0, &hit_loc);
    //assert(!result);

    result = collide_segment_solid_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(-2.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 4.0, &hit_loc);
    assert(result);
    assert(hit_loc == vector3d(-2.0f, 0.0f, 0.0f));

    result = collide_segment_solid_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 2.0, &hit_loc);
    assert(result);

    assert((hit_loc == vector3d(0.0f, 0.0f, 0.0f)) || (hit_loc == vector3d(1.0f, 0.0f, 0.0f)));

    sp_log("testing collide_segment_hollow_sphere\n");

    vector3d hit_locs[2];
    auto num_hits = collide_segment_hollow_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(0.0, 2.0, 0.0), vector3d(0.0, 2.0, 0.0), 1.0, hit_locs);

    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(0.0f, 1.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0), vector3d(-1.0, 0.0, 0.0), 1.0, hit_locs);
    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(0.0f, 0.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 0.0, hit_locs);
    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(0.0f, 0.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(-1.0, 0.0, 0.0), vector3d(-2.0, 0.0, 0.0), 1.0, hit_locs);
    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(-1.0f, 0.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(-1.0, 1.0, 0.0), vector3d(1.0, 1.0, 0.0), vector3d(1.0, 1.0, 0.0), 1.0, hit_locs);
    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(0.0f, 1.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(2.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 1.0, hit_locs);

    assert(num_hits == 2);
    assert(hit_locs[0] == vector3d(-1.0f, 0.0f, 0.0f));
    assert(hit_locs[1] == vector3d(1.0f, 0.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(2.0, 0.0, 0.0), vector3d(0.0, 1.0, 0.0), 1.0, hit_locs);
    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(0.0f, 0.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(-2.0, 2.0, 0.0), vector3d(2.0, 2.0, 0.0), vector3d(0.0, 0.0, 0.0), 1.0, hit_locs);
    assert(num_hits == 0);

    num_hits = collide_segment_hollow_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(-2.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 4.0, hit_locs);
    assert(num_hits == 0);

    num_hits = collide_segment_hollow_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(-2.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 2.0, hit_locs);
    assert(num_hits == 1);
    assert(hit_locs[0] == vector3d(-2.0f, 0.0f, 0.0f));

    num_hits = collide_segment_hollow_sphere(
        vector3d(-2.0, 0.0, 0.0), vector3d(-2.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 1.0, hit_locs);
    assert(num_hits == 0);
    num_hits = collide_segment_hollow_sphere(
        vector3d(0.0, 0.0, 0.0), vector3d(1.0, 0.0, 0.0), vector3d(0.0, 0.0, 0.0), 2.0, hit_locs);
    assert(num_hits == 0);
}

bool closest_point_segment(const vector3d &point, const vector3d &start, const vector3d &end, vector3d &closest)
{
    auto direction = end - start;
    const float length = direction.length();
    if (length > 0.0f)
        direction *= 1.0f / length;
    float distance = dot(point - start, direction);
    bool within_segment = true;
    if (distance < 0.0f) {
        distance = 0.0f;
        within_segment = false;
    } else if (distance > length) {
        distance = length;
        within_segment = false;
    }
    closest = start + direction * distance;
    return within_segment;
}

namespace {


vector3d swept_radius_extension(vector3d movement, float radius)
{
    const float length_squared = movement.length2();
    if (length_squared > 9.999999439624929e-11f)
        movement *= 1.0f / std::sqrt(length_squared);
    return movement * radius;
}


bool swept_segment_moving_entity(line_segment_t &segment, const entity &ent, const intraframe_trajectory_t &trajectory,
                                 float radius, vector3d &relative_start, vector3d &relative_end, vector3d &relative_hit)
{
    po initial = trajectory.world_po0;
    const po inverse_initial = *initial.inverse();
    po relative_transform;
    po::compose(relative_transform, inverse_initial, trajectory.world_po1);
    const po inverse_relative = *relative_transform.inverse();
    relative_start = inverse_initial.slow_xform(segment.field_0);
    relative_end = inverse_relative.slow_xform(inverse_initial.slow_xform(segment.field_C));
    relative_end += swept_radius_extension(relative_end - relative_start, radius);
    if (!collide_segment_entity(
            relative_start, relative_end, &ent, po_identity_matrix, &segment.field_18, &segment.field_24))
        return false;
    relative_hit = segment.field_18;
    segment.field_34 = true;
    segment.field_18 = initial.slow_xform(relative_transform.slow_xform(segment.field_18));
    segment.field_24 = initial.non_affine_slow_xform(relative_transform.non_affine_slow_xform(segment.field_24));
    return true;
}


bool swept_segment_moving_entities(local_collision::primitive_list_t *primitives, line_segment_t &segment,
                                   float duration, float radius, float *relative_start, float *relative_end,
                                   float *relative_hit, bool &selected_contact)
{
    float best_time = duration;
    bool collided = false;
    for (auto *primitive = primitives; primitive; primitive = primitive->field_0) {
        if (!primitive->is_ent || !primitive->field_10)
            continue;
        vector3d start, end, hit;
        if (!swept_segment_moving_entity(
                segment, *primitive->field_4.ent, *primitive->field_10, radius, start, end, hit))
            continue;
        const float length = (end - start).length();
        const float fraction = length >= EPSILON ? (hit - start).length() / length : 0.0f;
        const float time = fraction * duration;
        if (time < best_time) {
            best_time = time;
            relative_start[0] = start.x;
            relative_start[1] = start.y;
            relative_start[2] = start.z;
            relative_end[0] = end.x;
            relative_end[1] = end.y;
            relative_end[2] = end.z;
            relative_hit[0] = hit.x;
            relative_hit[1] = hit.y;
            relative_hit[2] = hit.z;
            selected_contact = true;
        }
        collided = true;
    }
    return collided;
}

struct swept_endpoint_contact {
    float distance_squared = 0.0f;
    float fraction = 0.0f;
    line_segment_t segment;
    bool valid = false;
};
}  // namespace


bool __fastcall swept_capsule_intersection(const capsule &end, const capsule &start, float duration, float radius,
                                           local_collision::primitive_list_t *primitives, float *time,
                                           local_collision::closest_points_pair_t *pair, bool *tunnelled,
                                           bool skip_base)
{
    line_segment_t paths[2]{{start.base, end.base}, {start.end, end.end}};
    const vector3d extensions[2]{swept_radius_extension(end.base - start.base, radius),
                                 swept_radius_extension(end.end - start.end, radius)};
    swept_endpoint_contact contacts[4];
    constexpr float tunnelling_tolerance = 9.99999905104687e-09f;


    float relative_start[3]{extensions[0].x, extensions[0].y, extensions[0].z};
    float relative_end[3];
    float relative_hit[3];
    bool selected_contact = false;
    for (int endpoint = skip_base ? 1 : 0; endpoint < 2; ++endpoint) {
        auto &path = paths[endpoint];
        path.ent = nullptr;
        path.field_34 = false;
        path.field_C += extensions[endpoint];
        auto &stationary = contacts[endpoint * 2];
        float distance;
        const float initial_time = 0.0f;
        if (local_collision::get_closest_line_intersection(primitives,
                                                           &path,
                                                           false,
                                                           &distance,
                                                           radius <= 0.0f && radius >= 0.0f ? &initial_time : nullptr,
                                                           nullptr)) {
            stationary.distance_squared = distance * distance;
            stationary.segment = path;
            const float length = (path.field_C - path.field_0).length() - radius;
            const vector3d original_movement = path.field_C - extensions[endpoint] - path.field_0;
            if (original_movement.length2() >= stationary.distance_squared - tunnelling_tolerance)
                *tunnelled = true;
            if (std::fabs(length) > EPSILON) {
                stationary.fraction = std::max((std::sqrt(stationary.distance_squared) - radius) / length, 0.0f);
                stationary.valid = true;
            }
        }
        path.field_C -= extensions[endpoint];
        auto &moving = contacts[endpoint * 2 + 1];
        if (radius > 0.0f &&
            swept_segment_moving_entities(
                primitives, path, duration, radius, relative_start, relative_end, relative_hit, selected_contact) &&
            selected_contact) {
            moving.segment = path;
            const vector3d local_start{relative_start[0], relative_start[1], relative_start[2]};
            const vector3d local_end{relative_end[0], relative_end[1], relative_end[2]};
            const vector3d local_hit{relative_hit[0], relative_hit[1], relative_hit[2]};
            moving.distance_squared = (local_start - local_hit).length2();
            const vector3d movement = local_end - local_start;
            const vector3d original_movement = movement - swept_radius_extension(movement, radius);
            if (original_movement.length2() >= moving.distance_squared - tunnelling_tolerance)
                *tunnelled = true;
            const float length = movement.length() - radius;
            if (length > EPSILON) {
                moving.fraction = std::max((std::sqrt(moving.distance_squared) - radius) / length, 0.0f);
                moving.valid = true;
            }
        }
    }
    const swept_endpoint_contact *best = nullptr;
    float earliest = std::numeric_limits<float>::max();
    for (const auto &contact : contacts) {
        if (contact.valid && contact.fraction >= 0.0f && contact.fraction < earliest) {
            best = &contact;
            earliest = contact.fraction;
        }
    }
    if (!best)
        return false;
    *time = duration * earliest;
    pair->point = pair->other_point = best->segment.field_18;
    pair->direction = pair->normal = best->segment.field_24;
    pair->distance_squared = best->distance_squared;
    return true;
}
