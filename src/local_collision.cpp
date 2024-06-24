#include "local_collision.h"

#include "actor.h"
#include "collide.h"
#include "collision_capsule.h"
#include "common.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "line_info.h"
#include "line_segment.h"
#include "loaded_regions_cache.h"
#include "oldmath_po.h"
#include "subdivision_node_obb_base.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cmath>

bool local_collision::entfilter_base::accept(
    actor *act,
    dynamic_conglomerate_clone *a2,
    const query_args_t &a3) const
{
    bool (__fastcall *func)(const void *, void *, actor *act,
        dynamic_conglomerate_clone *a2,
        const query_args_t *a3 ) = CAST(func, get_vfunc(m_vtbl, 0x0));
    return func(this, nullptr, act, a2, &a3);
}

bool find_intersection(const vector3d &a1,
                       const vector3d &a2,
                       const local_collision::entfilter_base &a3,
                       const local_collision::obbfilter_base &a4,
                       vector3d *point,
                       vector3d *normal,
                       region **a7,
                       entity **a8,
                       subdivision_node_obb_base **hit_obb,
                       bool a10)
{
    TRACE("find_intersection");
    
    if constexpr (0)
    {
        local_collision::query_args_t v23{};

        auto *v10 = local_collision::query_line_segment(a1, a2, a3, a4, v23);

        local_collision::intersection_list_t intersection_record{};

        line_segment_t a2a {a1, a2};

        auto v16 = local_collision::get_closest_line_intersection(v10,
                                                                  &a2a,
                                                                  a10,
                                                                  nullptr,
                                                                  nullptr,
                                                                  &intersection_record);

        if (v16) {
            assert(intersection_record.point.is_valid() && "get_closest_line_intersection failed");
            assert(intersection_record.normal.is_valid() && "get_closest_line_intersection failed");

            *point = intersection_record.point;
            *normal = intersection_record.normal;
            if (hit_obb == nullptr || intersection_record.is_ent) {
                if (a8 != nullptr && intersection_record.is_ent) {
                    auto *v17 = static_cast<entity *>(intersection_record.field_2C);
                    if (v17 == nullptr) {
                        v17 = static_cast<entity *>(intersection_record.intersection_node);
                    }

                    *a8 = v17;
                }

            } else {
                *hit_obb = static_cast<subdivision_node_obb_base *>(
                    intersection_record.intersection_node);

                assert((*hit_obb)->is_obb_node());
            }

            if (a7 != nullptr) {
                fixed_vector<region *, 15> regions;

                loaded_regions_cache::get_regions_intersecting_sphere(*point, 0.0f, &regions);

                *a7 = (regions.m_size != 0 ? regions.m_data[0] : nullptr);
            }
        }

        for (auto *i = v10; i != nullptr; i = i->field_0) {
            local_collision::primitive_list_t::pool.remove(i);
        }

        return v16;

    }
    else
    {
        bool (*func)(const vector3d *a1,
                       const vector3d *a2,
                       const local_collision::entfilter_base *a3,
                       const local_collision::obbfilter_base *a4,
                       vector3d *point,
                       vector3d *normal,
                       region **a7,
                       entity **a8,
                       subdivision_node_obb_base **hit_obb,
                       bool a10) = CAST(func, 0x005C4DD0);
        return func(&a1, &a2, &a3, &a4, point, normal, a7, a8, hit_obb, a10);
    }
}

void closest_point_line_segment_point(const vector3d &a1,
                                      const vector3d &a2,
                                      const vector3d &a3,
                                      float &a4)
{
    vector3d v5 = a2 - a1;
    auto v4 = closest_point_infinite_line_point(a1, v5, a3);

    a4 = std::clamp(v4, 0.0f, 1.0f);
}

namespace local_collision {

VALIDATE_SIZE(query_args_t, 0x34);

VALIDATE_SIZE(intersection_list_t, 0x30);

void local_collision::query_args_t::set_entity(entity *a2)
{
    this->field_2C = a2;
    this->initialized_flags |= 0x10u;
}

primitive_list_t::primitive_list_t(
        void *a2,
        void *a3)
{
    this->field_4.ent = static_cast<entity *>(a2);
    this->is_ent = true;
    this->field_8 = a3;
}

entity *primitive_list_t::get_entity() {
    assert(is_ent);

    return this->field_4.ent;
}

void * primitive_list_t::get_obb_node()
{
    assert(!is_ent);
    return this->field_4.obb;
}

bool test_line_intersection_ex(local_collision::primitive_list_t **a1,
                               const line_info &a2,
                               local_collision::primitive_list_t ***occluder) {
    assert(occluder != nullptr);
    *occluder = nullptr;

    local_collision::primitive_list_t **i = nullptr;
    for (i = a1;; i = (local_collision::primitive_list_t **) *i) {
        if (*i == nullptr) {
            return false;
        }

        bool v7 = false;
        vector3d a8{};
        vector3d a9{};
        if ((*i)->is_entity()) {
            auto &a7 = (*i)->field_4.ent->get_abs_po();
            v7 = collide_segment_entity(a2.field_0, a2.field_C, (*i)->field_4.ent, a7, &a8, &a9);
        } else {
            v7 = (*i)->field_4.obb->line_segment_intersection(a2.field_0, a2.field_C);
        }

        if (v7) {
            break;
        }
    }

    if (occluder != nullptr) {
        *occluder = i;
    }

    return true;
}

bool get_closest_line_intersection(local_collision::primitive_list_t *a1,
                                   line_segment_t *lif,
                                   bool a3,
                                   float *a4,
                                   const float *a5,
                                   local_collision::intersection_list_t *a6)
{
    TRACE("local_collision::get_closest_line_intersection");

    return (bool) CDECL_CALL(0x0053EE60, a1, lif, a3, a4, a5, a6);
}

primitive_list_t *query_sphere(const vector3d &a1,
                               Float a2,
                               const entfilter_base &a3,
                               const obbfilter_base &a4,
                               query_args_t query_args)
{
    TRACE("local_collision::query_sphere");

    if constexpr (0)
    {}
    else
    {
        return (primitive_list_t *) CDECL_CALL(0x005333C0, &a1, a2, &a3, &a4, query_args);
    }
}

bool get_closest_sphere_intersection(primitive_list_t *a1,
                                     const vector3d &a2,
                                     Float a3,
                                     vector3d *a4,
                                     vector3d *a5,
                                     intersection_list_t *best_intersection_record)
{
    TRACE("local_collision::get_closest_sphere_intersection");

    if constexpr (1)
    {
        local_collision::primitive_list_t *v7 = nullptr;
        float v20 = 3.4028235e38;

        vector3d point {};
        vector3d normal {};

        for (auto *it = a1; it != nullptr; it = it->field_0)
        {
            bool v9 = false;
            float arg10 = 0.0f;
            vector3d arg8 {};
            vector3d argC {};

            if (it->is_entity())
            {
                auto *ent = it->field_4.ent;

                auto &v25 = ent->get_abs_po();

                v9 = collide_sphere_entity(a2, a3, ent, &arg8, &argC, &v25);

                arg10 = dot((a2 - arg8), argC) - a3;

            }
            else
            {
                auto *obb = it->field_4.obb;

                assert(obb->is_obb_node());

                v9 = obb->sphere_intersection(a2, a3, &arg8, &argC, &arg10);
            }

            if (v9 && arg10 < v20)
            {
                v20 = arg10;
                point = arg8;
                normal = argC;
                v7 = it;
            }
        }

        if (v7 == nullptr) {
            return false;
        }

        *a4 = point;
        *a5 = normal;

        if (best_intersection_record != nullptr)
        {
            best_intersection_record->field_0 = 0;
            best_intersection_record->field_20 = 0;
            best_intersection_record->field_1C = (point - a2).length();
            best_intersection_record->is_ent = v7->is_entity();
            best_intersection_record->point = point;
            best_intersection_record->normal = normal;

            assert(best_intersection_record->point.is_valid() &&
                   "get_closest_sphere_intersection failed internally");

            assert(best_intersection_record->normal.is_valid() &&
                   "get_closest_sphere_intersection failed internally");

            if (v7->is_entity())
            {
                best_intersection_record->is_ent = true;
                best_intersection_record->intersection_node = v7->get_entity();
                best_intersection_record->field_2C = v7->field_8;
            }
            else
            {
                best_intersection_record->is_ent = false;
                best_intersection_record->intersection_node = v7->get_obb_node();
            }
        }

        return true;
    }
    else
    {
        bool (*func)(primitive_list_t *a1,
                     const vector3d *a2,
                     Float a3,
                     vector3d *a4,
                     vector3d *a5,
                     intersection_list_t *) = CAST(func, 0x00533660);
        return func(a1, &a2, a3, a4, a5, best_intersection_record);
    }
}

//0x00569E10
template<>
bool obbfilter<obbfilter_OBB_SPHERE_TEST>::accept(subdivision_node_obb_base *a1,
                                                  const query_args_t &a2) {
    return a1->sphere_intersection(a2.field_10, a2.field_28);
}

//0x00563810
template<>
bool entfilter<entfilter_AND<entfilter_ENTITY, entfilter_NO_CAPSULES>>::accept(
    actor *act,
    [[maybe_unused]] dynamic_conglomerate_clone *a2,
    [[maybe_unused]] const query_args_t &a3) {
    return act->has_entity_collision() && act->colgeom->get_type() != collision_geometry::CAPSULE;
}

bool sub_56CD20(actor *a1, dynamic_conglomerate_clone *a2, const local_collision::query_args_t &a3) {
    return a1->has_camera_collision() && a1->has_entity_collision() &&
        local_collision::entity_line_segment_test(a1, a2, a3);
}

//0x0056CD00
template<>
bool entfilter<local_collision::entfilter_AND<
    local_collision::entfilter_AND<local_collision::entfilter_COLLIDE_CAMERA,
                                   local_collision::entfilter_ENTITY>,
    local_collision::entfilter_LINESEG_TEST>>::accept(actor *a1,
                                                      dynamic_conglomerate_clone *a2,
                                                      const local_collision::query_args_t &a3) {
    return sub_56CD20(a1, a2, a3);
}

bool entity_line_segment_test(actor *a1,
                              dynamic_conglomerate_clone *a2,
                              const local_collision::query_args_t &a3) {
    return (bool) CDECL_CALL(0x0052F4E0, a1, a2, &a3);
}

void destroy_primitive_list(primitive_list_t **a1) {
    CDECL_CALL(0x00510720, a1);
}

primitive_list_t *query_line_segment(const vector3d &a1,
                                     const vector3d &a2,
                                     const local_collision::entfilter_base &a3,
                                     const local_collision::obbfilter_base &a4,
                                     local_collision::query_args_t a5)
{
    TRACE("local_collision::query_line_segment");

    return (primitive_list_t *) CDECL_CALL(0x00533260, &a1, &a2, &a3, &a4, a5);
}

} // namespace local_collision



bool sub_50D220(const vector3d &a1, const vector3d &a2, entity *a3)
{
    entity *a8 = nullptr;
    vector3d point, normal;
    return !find_intersection(
        a1,
        a2,
        *local_collision::entfilter_blocks_beams,
        *local_collision::obbfilter_lineseg_test,
        &point,
        &normal,
        nullptr,
        &a8,
        nullptr,
        false)
        || (a3 != nullptr && a3 == a8);
}

bool local_collision::collision_pair_matches_query_constraints(
        actor *a1,
        dynamic_conglomerate_clone *a2,
        local_collision::entfilter_base &a3,
        local_collision::query_args_t &a4)
{
    return a1->get_colgeom() != nullptr
          && a3.accept(a1, a2, a4)
          && a1->are_collisions_active();
}

template<>
local_collision::entfilter<local_collision::entfilter_AND<local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,local_collision::entfilter_NO_CAPSULES>,local_collision::entfilter_AND<local_collision::entfilter_ENTITY,walkable_entfilter_t>>>::entfilter()
{
    this->m_vtbl = 0x008895A8;
}

template<>
local_collision::obbfilter<local_collision::obbfilter_AND<walkable_obbfilter_t,local_collision::obbfilter_OBB_LINE_SEGMENT_TEST>>::obbfilter()
{
    this->m_vtbl = 0x008895AC;
}

template<>
local_collision::entfilter<local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,local_collision::entfilter_VALID_COLLISION_PAIR>>::entfilter()
{
    this->m_vtbl = 0x00892C3C;
}

template<>
local_collision::entfilter<local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,local_collision::entfilter_AND<local_collision::entfilter_VALID_COLLISION_PAIR,local_collision::entfilter_SPHERE_TEST>>>::entfilter()
{
    this->m_vtbl = 0x00892C38;
}


void local_collision_patch()
{
    REDIRECT(0x0052F009, find_intersection);
}

