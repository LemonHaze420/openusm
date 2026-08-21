#include "collide.h"

#include "collision_capsule.h"
#include "colmesh.h"
#include "colmesh_common.h"
#include "custom_math.h"
#include "entity.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "local_collision.h"
#include "log.h"
#include "mesh_triangle_intersection_record.h"
#include "ngl_math.h"
#include "oldmath_po.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "vector3d.h"

#include <cassert>
#include <cmath>

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

bool best_sphere_obb_tree_intersection(math::VecClass<3, 0> const &a1, const cg_mesh *a2, const collision_obb_t &a3,
                                       math::VecClass<3, 0> &a4, float &a5)
{
    if constexpr (0) {
    } else {
        return (bool)CDECL_CALL(0x005CA780, &a1, a2, &a3, &a4, &a5);
    }
}

bool collide_sphere_mesh(const vector3d &a1, Float a2, const cg_mesh *mesh, vector3d *hit_loc, vector3d *hit_norm)
{
    if constexpr (0) {
    } else {
        return CDECL_CALL(0x005CB2D0, &a1, a2, mesh, hit_loc, hit_norm);
    }
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

bool collide_segment_capsule(const vector3d &a1, const vector3d &a2, const vector3d &a3, const vector3d &a4, Float a5,
                             vector3d *hit_point, vector3d *hit_normal)
{
    if constexpr (0) {
        assert(hit_point != nullptr && hit_normal != nullptr);

    } else {
        return (bool)CDECL_CALL(0x005C4A20, &a1, &a2, &a3, &a4, a5, hit_point, hit_normal);
    }
}

vector3d sub_580BB0(float *self, const uint16_t *a3)
{
    auto *v3 = &self[3 * a3[0] + 1];
    auto v4 = self[3 * a3[2] + 1] - *v3;
    auto v5 = self[3 * a3[2] + 2] - self[3 * a3[0] + 2];
    auto v6 = self[3 * a3[2] + 3] - self[3 * a3[0] + 3];
    auto *v7 = &self[3 * a3[1] + 1];
    auto v10 = v7[0] - v3[0];
    auto v11 = v7[1] - v3[1];
    auto v8 = v7[2] - v3[2];

    vector3d v12;
    v12[0] = v11 * v6 - v8 * v5;
    v12[1] = v8 * v4 - v6 * v10;
    v12[2] = v10 * v5 - v11 * v4;

    return v12;
}

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
    auto v10 = v5->field_10;
    auto *v11 = mesh->data;
    int v12 = v11->field_10[v10].field_35;
    auto *v13 = &v11->field_10[v10];

    vector3d v20 = sub_580BB0(
        (float *)((char *)&v13->field_0[0x4000 * v12] + v13->field_36),

        (unsigned __int16
             *)((char *)&v13->field_0[0x4000 * v12 + 1 +
                                      3 * *(unsigned __int16 *)((char *)&v13->field_0[0x4000 * v12] + v13->field_36)] +
                6 * v5->field_C + v13->field_36));

    *closest_normal = v20;

    auto v16 = closest_normal->length2();

    if (v16 > 9.9999994e-11) {
        auto v18 = 1.f / sqrt(v16);
        *closest_normal *= v18;
    }

    return true;
}

void line_segment_obb_tree_intersection(const vector3d &a1, const vector3d &a2, const cg_mesh *a3,
                                        const collision_obb_t *a4, mesh_triangle_intersection_record_t **a5)
{
    CDECL_CALL(0x005CA5D0, &a1, &a2, a3, a4, a5);
}

bool collide_segment_mesh(const vector3d &a3, const vector3d &a2, cg_mesh *mesh, vector3d *closest_point,
                          vector3d *closest_normal)
{
    assert(mesh != nullptr);

    auto *v11 = mesh->data->field_10;
    mesh_triangle_intersection_record_t *v12 = nullptr;
    line_segment_obb_tree_intersection(a3, a2, mesh, v11, &v12);
    auto result = get_closest_intersection_from_list(a3, mesh, v12, closest_point, closest_normal);

    if (v12 != nullptr) {
        mesh_triangle_intersection_record_t::pool().remove(v12->field_14);
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

bool collide_segment_entity(const vector3d &a2, const vector3d &a3, const entity *a4, const po &a5, vector3d *a6,
                            vector3d *a7)
{
    if constexpr (1) {
        auto *v6 = a4->get_colgeom();
        auto a4a = a4->get_colgeom_radius();
        auto v7 = v6->get_local_space_bounding_sphere_center();

        auto v11 = a5.m * v7;
        if (a4a * a4a >= dist_point_segment_sq_opt(v11, a2, a3)) {
            return collide_segment_geometry(a2, a3, v6, a5, a6, a7);
        }

        return false;

    } else {
        return CDECL_CALL(0x005CB740, &a2, &a3, a4, &a5, a6, a7);
    }
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
    assert(radius1 > EPSILON && radius2 > EPSILON);

    return (bool)CDECL_CALL(0x005C47A0, &a1, &a2, radius1, &a4, &a5, radius2, &cp1, &cp2, &normal);
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

bool closest_point_segment(const vector3d &a1, const vector3d &a2, const vector3d &a3, vector3d &a4)
{
    return (bool)CDECL_CALL(0x005B99F0, &a1, &a2, &a3, &a4);
}
