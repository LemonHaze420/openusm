#include "local_collision.h"

#include "actor.h"
#include "collide.h"
#include "collision_capsule.h"
#include "colmesh.h"
#include "common.h"
#include "conglom.h"
#include "dynamic_rtree.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "intraframe_trajectory.h"
#include "line_info.h"
#include "line_segment.h"
#include "loaded_regions_cache.h"
#include "oldmath_po.h"
#include "oriented_bounding_box_root_node.h"
#include "region.h"
#include "rtree.h"
#include "subdivision_visitor.h"
#include "subdivision_obb.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

#include <cmath>

bool local_collision::entfilter_base::accept(actor *act, dynamic_conglomerate_clone *a2, const query_args_t &a3) const
{
    return reinterpret_cast<const native_vtable *>(m_vtbl)->accept(this, nullptr, act, a2, &a3);
}

bool local_collision::obbfilter_base::accept(subdivision_node_obb_base *node, const query_args_t &args) const
{
    return reinterpret_cast<const native_vtable *>(m_vtbl)->accept(this, nullptr, node, &args);
}

namespace local_collision {
namespace {

template <bool Value>
bool __fastcall constant_entity_filter(const entfilter_base *, void *, actor *, dynamic_conglomerate_clone *,
                                       const query_args_t *)
{
    return Value;
}

template <bool Value>
bool __fastcall constant_obb_filter(const obbfilter_base *, void *, subdivision_node_obb_base *, const query_args_t *)
{
    return Value;
}

bool __fastcall line_obb_filter(const obbfilter_base *, void *, subdivision_node_obb_base *node,
                                const query_args_t *args)
{
    return node->line_segment_intersection(args->field_4, args->field_1C);
}

bool __fastcall sphere_obb_filter(const obbfilter_base *, void *, subdivision_node_obb_base *node,
                                  const query_args_t *args)
{
    return node->sphere_intersection(args->field_10, args->field_28);
}

bool __fastcall walkable_obb_filter(const obbfilter_base *self, void *unused, subdivision_node_obb_base *node,
                                    const query_args_t *args)
{
    return (node->flags & 0x50) != 0x50 && line_obb_filter(self, unused, node, args);
}

bool __fastcall walkable_entity_filter(const entfilter_base *, void *, actor *act, dynamic_conglomerate_clone *,
                                       const query_args_t *args)
{
    return act != args->field_2C && act->colgeom->get_type() != collision_geometry::CAPSULE &&
           act->has_entity_collision() && (act->possibly_walkable() || act->sub_4C08E0());
}

bool __fastcall entity_no_capsules_filter(const entfilter_base *, void *, actor *act, dynamic_conglomerate_clone *,
                                          const query_args_t *)
{
    return act->has_entity_collision() && act->colgeom->get_type() != collision_geometry::CAPSULE;
}

bool __fastcall blocks_ai_los_filter(const entfilter_base *, void *, actor *act, dynamic_conglomerate_clone *clone,
                                     const query_args_t *args)
{
    if (!entity_line_segment_test(act, clone, *args))
        return false;
    bool ai_actor = act->get_ai_core() != nullptr && (act->field_4 & 0x800) == 0;
    if (!ai_actor && (act->field_4 & (0x8000 | 4)) != 0) {
        auto *owner = act->get_conglom_owner();
        ai_actor = owner->get_ai_core() != nullptr && (owner->field_4 & 0x800) == 0;
    }
    return (act->field_8 & 0x100000) == 0 && !ai_actor;
}

bool __fastcall valid_pair_filter(const entfilter_base *, void *, actor *act, dynamic_conglomerate_clone *,
                                  const query_args_t *args)
{
    auto *other = static_cast<actor *>(args->field_30);
    return act != args->field_2C && (other->field_4 & 0x4000) != 0 &&
           (act->colgeom->get_type() != collision_geometry::CAPSULE ||
            other->colgeom->get_type() != collision_geometry::CAPSULE ||
            (act->are_character_collisions_active() && other->are_character_collisions_active())) &&
           act->allow_collision(other->my_handle) && other->allow_collision(act->my_handle);
}

bool __fastcall valid_pair_sphere_filter(const entfilter_base *self, void *unused, actor *act,
                                         dynamic_conglomerate_clone *clone, const query_args_t *args)
{
    if (!valid_pair_filter(self, unused, act, clone, args))
        return false;
    if ((act->field_4 & 4) != 0) {
        const float radius = act->get_colgeom_radius() + args->field_28;
        return (act->get_colgeom_center() - args->field_10).length2() < radius * radius;
    }
    const auto center = act->get_abs_po().inverse_xform(args->field_10);
    if (act->colgeom->get_type() == collision_geometry::MESH) {
        const auto &box = static_cast<const cg_mesh *>(act->colgeom)->data->field_10[0];
        const auto relative = center - box.field_0;
        const vector3d *axes[]{&box.field_10, &box.axis_y, &box.axis_z};
        float distance_squared = 0.0f;
        for (const auto *axis : axes) {
            const float extent = std::sqrt(axis->length2());
            const float coordinate = std::fabs(dot(*axis, relative) / extent);
            const float distance = coordinate > extent ? coordinate - extent : 0.0f;
            distance_squared += distance * distance;
        }
        return distance_squared <= args->field_28 * args->field_28;
    }
    const float radius = act->colgeom->get_bounding_sphere_radius() + args->field_28;
    return (center - act->colgeom->get_local_space_bounding_sphere_center()).length2() < radius * radius;
}

const entfilter_base::native_vtable valid_pair_table{valid_pair_filter};
const entfilter_base::native_vtable valid_pair_sphere_table{valid_pair_sphere_filter};
const entfilter_base::native_vtable reject_entity_table{constant_entity_filter<false>};
const entfilter_base::native_vtable accept_entity_table{constant_entity_filter<true>};
const entfilter_base::native_vtable walkable_entity_table{walkable_entity_filter};
const entfilter_base::native_vtable entity_no_capsules_table{entity_no_capsules_filter};
const entfilter_base::native_vtable blocks_ai_los_table{blocks_ai_los_filter};
const obbfilter_base::native_vtable reject_obb_table{constant_obb_filter<false>};
const obbfilter_base::native_vtable accept_obb_table{constant_obb_filter<true>};
const obbfilter_base::native_vtable line_obb_table{line_obb_filter};
const obbfilter_base::native_vtable sphere_obb_table{sphere_obb_filter};
const obbfilter_base::native_vtable walkable_obb_table{walkable_obb_filter};

entfilter_reject_all_t reject_entity_instance{{reinterpret_cast<std::intptr_t>(&reject_entity_table)}};
entfilter_accept_all_t accept_entity_instance{{reinterpret_cast<std::intptr_t>(&accept_entity_table)}};
entfilter_base blocks_ai_los_instance{reinterpret_cast<std::intptr_t>(&blocks_ai_los_table)};
obbfilter_base reject_obb_instance{reinterpret_cast<std::intptr_t>(&reject_obb_table)};
obbfilter_base accept_obb_instance{reinterpret_cast<std::intptr_t>(&accept_obb_table)};
obbfilter_base line_obb_instance{reinterpret_cast<std::intptr_t>(&line_obb_table)};
obbfilter_base sphere_obb_instance{reinterpret_cast<std::intptr_t>(&sphere_obb_table)};

template <typename T>
T *&bind_filter(std::uintptr_t address, T &instance)
{
    auto &pointer = var<T *>(address);
    pointer = &instance;
    return pointer;
}

primitive_list_t *allocate_primitive()
{
    static const bool initialized =
        (primitive_list_t::pool.init(sizeof(primitive_list_t), 128, 4, 1, 0, nullptr), true);
    (void)initialized;
    return static_cast<primitive_list_t *>(primitive_list_t::pool.allocate_new_block());
}

struct entity_local_collision_visitor : subdivision_visitor {
    primitive_list_t **head;
    const entfilter_base *filter;
    const query_args_t *args;

    void add(entity *ent)
    {
        auto *entry = allocate_primitive();
        entry->field_0 = *head;
        entry->field_4.ent = ent;
        entry->field_8 = nullptr;
        entry->is_ent = true;
        entry->field_10 = nullptr;
        *head = entry;
    }

    static int visit_node(subdivision_visitor &base, const subdivision_node &node)
    {
        auto &self = static_cast<entity_local_collision_visitor &>(base);
        auto *act = reinterpret_cast<actor *>(const_cast<subdivision_node *>(&node));
        if (act->field_64 == entity::visit_key3)
            return 0;
        act->field_64 = entity::visit_key3;
        if (act->colgeom && self.filter->accept(act, nullptr, *self.args) && act->are_collisions_active())
            self.add(act);
        if (act->is_a_conglomerate()) {
            auto *children = static_cast<conglomerate *>(act)->field_FC;
            if (children) {
                for (auto *child : *children) {
                    if (child->colgeom && self.filter->accept(child, nullptr, *self.args) &&
                        child->are_collisions_active() && child->field_64 != entity::visit_key3) {
                        self.add(child);
                        child->field_64 = entity::visit_key3;
                    }
                }
            }
        }
        return 0;
    }

    entity_local_collision_visitor(primitive_list_t **list, const entfilter_base &f, const query_args_t &a)
        : head(list), filter(&f), args(&a)
    {
        static const native_vtable table{visit_node, nullptr};
        m_vtbl = reinterpret_cast<std::intptr_t>(&table);
    }
};

struct terrain_local_collision_visitor : subdivision_visitor {
    primitive_list_t **head;
    const obbfilter_base *filter;
    const query_args_t *args;

    static int visit_node(subdivision_visitor &base, const subdivision_node &node)
    {
        auto &self = static_cast<terrain_local_collision_visitor &>(base);
        auto *obb = static_cast<subdivision_node_obb_base *>(const_cast<subdivision_node *>(&node));
        if (self.filter->accept(obb, *self.args)) {
            auto *entry = allocate_primitive();
            entry->field_0 = *self.head;
            entry->field_4.obb = obb;
            entry->field_8 = nullptr;
            entry->is_ent = false;
            entry->field_10 = nullptr;
            *self.head = entry;
        }
        return 0;
    }

    terrain_local_collision_visitor(primitive_list_t **list, const obbfilter_base &f, const query_args_t &a)
        : head(list), filter(&f), args(&a)
    {
        static const native_vtable table{visit_node, nullptr};
        m_vtbl = reinterpret_cast<std::intptr_t>(&table);
    }
};
}  // namespace

entfilter_reject_all_t *&entfilter_reject_all = bind_filter(0x00960054, reject_entity_instance);
entfilter_accept_all_t *&entfilter_accept_all = bind_filter(0x0096004C, accept_entity_instance);
entfilter_base *&entfilter_blocks_ai_los = bind_filter(0x0096005C, blocks_ai_los_instance);
entfilter<entfilter_AND<entfilter_ENTITY, entfilter_NO_CAPSULES>> *&entfilter_entity_no_capsules =
    bind_filter(0x00960068, entfilter_entity_no_capsules_instance);
obbfilter_base *&obbfilter_lineseg_test = bind_filter(0x00960064, line_obb_instance);
obbfilter_base *&obbfilter_sphere_test = bind_filter(0x00960050, sphere_obb_instance);
obbfilter_base *&obbfilter_reject_all = bind_filter(0x0096006C, reject_obb_instance);
obbfilter_base *&obbfilter_accept_all = bind_filter(0x00960048, accept_obb_instance);
}  // namespace local_collision

bool find_intersection(const vector3d &start, const vector3d &end, const local_collision::entfilter_base &entity_filter,
                       const local_collision::obbfilter_base &terrain_filter, vector3d *point, vector3d *normal,
                       region **out_region, entity **out_entity, subdivision_node_obb_base **out_obb, bool two_sided)
{
    local_collision::query_args_t arguments{};
    auto *primitives = local_collision::query_line_segment(start, end, entity_filter, terrain_filter, arguments);
    local_collision::intersection_list_t intersection{};
    line_segment_t segment{start, end};
    const bool hit = local_collision::get_closest_line_intersection(
        primitives, &segment, two_sided, nullptr, nullptr, &intersection);
    if (hit) {
        *point = intersection.point;
        *normal = intersection.normal;
        if (out_obb != nullptr && !intersection.is_ent)
            *out_obb = static_cast<subdivision_node_obb_base *>(intersection.intersection_node);
        else if (out_entity != nullptr && intersection.is_ent)
            *out_entity = static_cast<entity *>(intersection.field_2C != nullptr ? intersection.field_2C
                                                                                 : intersection.intersection_node);
        if (out_region != nullptr) {
            fixed_vector<region *, 15> regions;
            loaded_regions_cache::get_regions_intersecting_sphere(*point, 0.0f, &regions);
            *out_region = regions.m_size != 0 ? regions.m_data[0] : nullptr;
        }
    }
    while (primitives != nullptr) {
        auto *next = primitives->field_0;
        local_collision::primitive_list_t::pool.remove(primitives);
        primitives = next;
    }
    return hit;
}

void closest_point_line_segment_point(const vector3d &a1, const vector3d &a2, const vector3d &a3, float &a4)
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

primitive_list_t::primitive_list_t(void *a2, void *a3)
{
    this->field_4.ent = static_cast<entity *>(a2);
    this->is_ent = true;
    this->field_8 = a3;
}

entity *primitive_list_t::get_entity()
{
    assert(is_ent);

    return this->field_4.ent;
}

void *primitive_list_t::get_obb_node()
{
    assert(!is_ent);
    return this->field_4.obb;
}

bool test_line_intersection_ex(local_collision::primitive_list_t **a1, const line_info &a2,
                               local_collision::primitive_list_t ***occluder)
{
    assert(occluder != nullptr);
    *occluder = nullptr;

    local_collision::primitive_list_t **i = nullptr;
    for (i = a1;; i = (local_collision::primitive_list_t **)*i) {
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

bool get_closest_line_intersection(primitive_list_t *primitives, line_segment_t *lif, bool allow_exit, float *distance,
                                   const float *time, intersection_list_t *record)
{
    const vector3d start = lif->field_0;
    vector3d end = lif->field_C;
    vector3d normal;
    primitive_list_t *closest = nullptr;
    for (auto *candidate = primitives; candidate; candidate = candidate->field_0) {
        vector3d hit_point, hit_normal;
        bool hit;
        if (candidate->is_ent) {
            auto *ent = candidate->field_4.ent;
            if (candidate->field_10) {
                if (!time)
                    continue;
                po transform;
                candidate->field_10->integrate(*time, &transform);
                hit = collide_segment_entity(start, end, ent, transform, &hit_point, &hit_normal);
            } else {
                hit = collide_segment_entity(start, end, ent, ent->get_abs_po(), &hit_point, &hit_normal);
            }
        } else {
            hit = candidate->field_4.obb->line_segment_intersection(
                start, end, &hit_point, &hit_normal, nullptr, allow_exit);
        }
        if (hit) {
            closest = candidate;
            end = hit_point;
            normal = hit_normal;
        }
    }
    if (!closest) {
        lif->field_34 = false;
        return false;
    }
    lif->field_24 = normal;
    lif->field_18 = end;
    lif->field_34 = true;
    lif->ent = nullptr;
    const float hit_distance = (end - start).length();
    if (distance)
        *distance = hit_distance;
    if (record) {
        record->field_0 = 0;
        record->field_20 = 0;
        record->field_1C = hit_distance;
        record->is_ent = closest->is_ent;
        record->point = end;
        record->normal = normal;
        if (closest->is_ent) {
            record->intersection_node = closest->field_4.ent;
            record->field_2C = closest->field_8;

            lif->ent = closest->field_4.ent;
        } else {
            record->intersection_node = closest->field_4.obb;
        }
    }
    return true;
}

primitive_list_t *query_sphere(const vector3d &center, Float radius, const entfilter_base &entity_filter,
                               const obbfilter_base &terrain_filter, query_args_t args)
{
    args.field_10 = center;
    args.field_28 = radius;
    args.initialized_flags |= 0xC;
    fixed_vector<region *, 15> regions;
    loaded_regions_cache::get_regions_intersecting_sphere(center, std::max(float(radius), 20.0f), &regions);
    primitive_list_t *head = nullptr;
    const vector3d minimum = center - float(radius);
    const vector3d maximum = center + float(radius);
    if (&terrain_filter != obbfilter_reject_all) {
        terrain_local_collision_visitor visitor(&head, terrain_filter, args);
        ++subdivision_node_obb_base::visit_key();
        for (auto *reg : regions) {
            if (reg->field_98)
                traverse_rtree(minimum, maximum, *reg->field_98, visitor);
        }
    }
    if (&entity_filter != entfilter_reject_all) {
        entity_local_collision_visitor visitor(&head, entity_filter, args);
        ++entity::visit_key3;
        traverse_rtree(minimum, maximum, collision_dynamic_rtree().state->field_110, visitor);
    }
    return head;
}

closest_points_pair_t *allocate_closest_points_pair()
{
    static const bool initialized =
        (closest_points_pair_t::pool.init(sizeof(closest_points_pair_t), 128, 4, 1, 0, nullptr), true);
    (void)initialized;
    auto *pair = static_cast<closest_points_pair_t *>(closest_points_pair_t::pool.allocate_new_block());
    pair->next = nullptr;
    pair->primitive = nullptr;
    return pair;
}

void destroy_closest_points_pair_list(closest_points_pair_t **pairs)
{
    while (*pairs) {
        auto *pair = *pairs;
        *pairs = pair->next;
        closest_points_pair_t::pool.remove(pair);
    }
}

closest_points_pair_t *get_all_capsule_intersections(primitive_list_t *primitives, const capsule &query, float time)
{
    closest_points_pair_t *head = nullptr;
    auto **tail = &head;
    for (auto *primitive = primitives; primitive; primitive = primitive->field_0) {
        if (primitive->is_ent) {
            po transform;
            if (primitive->field_10)
                primitive->field_10->integrate(time, &transform);
            else
                transform = primitive->field_4.ent->get_abs_po();
            auto *pairs = collide_capsule_entity(query, primitive->field_4.ent, transform);
            *tail = pairs;
            for (auto *pair = pairs; pair; pair = pair->next) {
                pair->primitive = primitive;
                tail = &pair->next;
            }
        } else {
            closest_points_pair_t contact{};
            if (primitive->field_4.obb->capsule_intersection(query, &contact)) {
                auto *pair = allocate_closest_points_pair();
                *pair = contact;
                pair->next = head;
                pair->primitive = primitive;
                if (!head)
                    tail = &pair->next;
                head = pair;
            }
        }
    }
    return head;
}

namespace {
fixed_pool &sphere_intersection_pool()
{
    auto &pool = var<fixed_pool>(0x00922198);
    static const bool initialized = (pool.init(sizeof(intersection_list_t), 128, 4, 1, 0, nullptr), true);
    (void)initialized;
    return pool;
}
}  // namespace

intersection_list_t *get_all_sphere_intersections(primitive_list_t *primitives, const vector3d &center, Float radius)
{
    intersection_list_t *head = nullptr;
    for (auto *primitive = primitives; primitive != nullptr; primitive = primitive->field_0) {
        vector3d point, normal;
        float penetration;
        bool hit;
        if (primitive->is_ent) {
            auto *owner = primitive->field_4.ent;
            po pose = owner->get_abs_po();
            hit = collide_sphere_entity(center, radius, owner, &point, &normal, &pose);
            if (hit)
                penetration = dot(center - point, normal) - radius;
        } else {
            hit = primitive->field_4.obb->sphere_intersection(center, radius, &point, &normal, &penetration);
        }
        if (!hit)
            continue;
        auto *intersection = static_cast<intersection_list_t *>(sphere_intersection_pool().allocate_new_block());
        assert(intersection != nullptr);
        intersection->field_0 = reinterpret_cast<int>(head);
        intersection->normal = normal;
        intersection->point = point;
        intersection->field_1C = penetration;
        intersection->field_20 = 0;
        intersection->is_ent = primitive->is_ent;
        intersection->intersection_node = primitive->is_ent ? static_cast<void *>(primitive->field_4.ent)
                                                            : static_cast<void *>(primitive->field_4.obb);
        if (primitive->is_ent)
            intersection->field_2C = primitive->field_8;
        head = intersection;
    }
    return head;
}

void destroy_intersection_list(intersection_list_t **intersections)
{
    while (*intersections != nullptr) {
        auto *current = *intersections;
        *intersections = reinterpret_cast<intersection_list_t *>(current->field_0);
        sphere_intersection_pool().remove(current);
    }
}

bool get_closest_sphere_intersection(primitive_list_t *a1, const vector3d &a2, Float a3, vector3d *a4, vector3d *a5,
                                     intersection_list_t *best_intersection_record)
{
    TRACE("local_collision::get_closest_sphere_intersection");

    if constexpr (1) {
        local_collision::primitive_list_t *v7 = nullptr;
        float v20 = 3.4028235e38;

        vector3d point{};
        vector3d normal{};

        for (auto *it = a1; it != nullptr; it = it->field_0) {
            bool v9 = false;
            float arg10 = 0.0f;
            vector3d arg8{};
            vector3d argC{};

            if (it->is_entity()) {
                auto *ent = it->field_4.ent;

                auto &v25 = ent->get_abs_po();

                v9 = collide_sphere_entity(a2, a3, ent, &arg8, &argC, &v25);

                arg10 = dot((a2 - arg8), argC) - a3;

            } else {
                auto *obb = it->field_4.obb;

                assert(obb->is_obb_node());

                v9 = obb->sphere_intersection(a2, a3, &arg8, &argC, &arg10);
            }

            if (v9 && arg10 < v20) {
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

        if (best_intersection_record != nullptr) {
            best_intersection_record->field_0 = 0;
            best_intersection_record->field_20 = 0;
            best_intersection_record->field_1C = (point - a2).length();
            best_intersection_record->is_ent = v7->is_entity();
            best_intersection_record->point = point;
            best_intersection_record->normal = normal;

            assert(best_intersection_record->point.is_valid() && "get_closest_sphere_intersection failed internally");

            assert(best_intersection_record->normal.is_valid() && "get_closest_sphere_intersection failed internally");

            if (v7->is_entity()) {
                best_intersection_record->is_ent = true;
                best_intersection_record->intersection_node = v7->get_entity();
                best_intersection_record->field_2C = v7->field_8;
            } else {
                best_intersection_record->is_ent = false;
                best_intersection_record->intersection_node = v7->get_obb_node();
            }
        }

        return true;
    } else {
        bool (*func)(
            primitive_list_t *a1, const vector3d *a2, Float a3, vector3d *a4, vector3d *a5, intersection_list_t *) =
            CAST(func, 0x00533660);
        return func(a1, &a2, a3, a4, a5, best_intersection_record);
    }
}

//0x00569E10
template <>
bool obbfilter<obbfilter_OBB_SPHERE_TEST>::accept(subdivision_node_obb_base *a1, const query_args_t &a2)
{
    return a1->sphere_intersection(a2.field_10, a2.field_28);
}

//0x00563810
template <>
bool entfilter<entfilter_AND<entfilter_ENTITY, entfilter_NO_CAPSULES>>::accept(
    actor *act, [[maybe_unused]] dynamic_conglomerate_clone *a2, [[maybe_unused]] const query_args_t &a3)
{
    return act->has_entity_collision() && act->colgeom->get_type() != collision_geometry::CAPSULE;
}

bool sub_56CD20(actor *a1, dynamic_conglomerate_clone *a2, const local_collision::query_args_t &a3)
{
    return a1->has_camera_collision() && a1->has_entity_collision() &&
           local_collision::entity_line_segment_test(a1, a2, a3);
}

//0x0056CD00
template <>
bool entfilter<local_collision::entfilter_AND<
    local_collision::entfilter_AND<local_collision::entfilter_COLLIDE_CAMERA, local_collision::entfilter_ENTITY>,
    local_collision::entfilter_LINESEG_TEST>>::accept(actor *a1, dynamic_conglomerate_clone *a2,
                                                      const local_collision::query_args_t &a3)
{
    return sub_56CD20(a1, a2, a3);
}

bool entity_line_segment_test(actor *owner, dynamic_conglomerate_clone *, const local_collision::query_args_t &args)
{
    if ((owner->field_4 & 4u) != 0) {
        const auto center = owner->get_colgeom_center();
        float parameter;
        closest_point_line_segment_point(args.field_4, args.field_1C, center, parameter);
        const auto closest = args.field_4 + (args.field_1C - args.field_4) * parameter;
        const float radius = owner->get_colgeom_radius();
        return radius * radius > (closest - center).length2();
    }
    const auto &pose = owner->get_abs_po();
    const auto start = pose.inverse_xform(args.field_4);
    const auto end = pose.inverse_xform(args.field_1C);
    if (owner->colgeom->get_type() == collision_geometry::MESH) {
        const auto *mesh = static_cast<const cg_mesh *>(owner->colgeom);
        return segment_mesh_box_overlap(start, end, mesh->data->field_10[0]);
    }
    const float radius = owner->colgeom->get_bounding_sphere_radius();
    const auto center = owner->colgeom->get_local_space_bounding_sphere_center();
    float parameter;
    closest_point_line_segment_point(start, end, center, parameter);
    const auto closest = args.field_4 + (args.field_1C - args.field_4) * parameter;
    return radius * radius > (closest - center).length2();
}

void destroy_primitive_list(primitive_list_t **head)
{
    while (*head) {
        auto *entry = *head;
        *head = entry->field_0;
        primitive_list_t::pool.remove(entry);
    }
}

primitive_list_t *query_line_segment(const vector3d &start, const vector3d &end, const entfilter_base &entity_filter,
                                     const obbfilter_base &terrain_filter, query_args_t args)
{
    args.field_4 = start;
    args.field_1C = end;
    args.initialized_flags |= 3;
    primitive_list_t *head = nullptr;
    fixed_vector<region *, 15> regions;
    loaded_regions_cache::get_regions_intersecting_box(start, end, &regions, vector3d{20.0f, 20.0f, 20.0f});
    if (&entity_filter != entfilter_reject_all) {
        entity_local_collision_visitor visitor(&head, entity_filter, args);
        ++entity::visit_key3;
        traverse_rtree(start, end, collision_dynamic_rtree().state->field_110, visitor);
    }
    if (&terrain_filter != obbfilter_reject_all) {
        terrain_local_collision_visitor visitor(&head, terrain_filter, args);
        ++subdivision_node_obb_base::visit_key();
        for (auto *reg : regions) {
            if (reg->field_98)
                traverse_rtree(start, end, *reg->field_98, visitor);
        }
    }
    return head;
}

}  // namespace local_collision


bool sub_50D220(const vector3d &a1, const vector3d &a2, entity *a3)
{
    entity *a8 = nullptr;
    vector3d point, normal;
    return !find_intersection(a1,
                              a2,
                              *local_collision::entfilter_blocks_beams,
                              *local_collision::obbfilter_lineseg_test,
                              &point,
                              &normal,
                              nullptr,
                              &a8,
                              nullptr,
                              false) ||
           (a3 != nullptr && a3 == a8);
}

bool local_collision::collision_pair_matches_query_constraints(actor *a1, dynamic_conglomerate_clone *a2,
                                                               local_collision::entfilter_base &a3,
                                                               local_collision::query_args_t &a4)
{
    return a1->get_colgeom() != nullptr && a3.accept(a1, a2, a4) && a1->are_collisions_active();
}

template <>
local_collision::entfilter<local_collision::entfilter_AND<
    local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY, local_collision::entfilter_NO_CAPSULES>,
    local_collision::entfilter_AND<local_collision::entfilter_ENTITY, walkable_entfilter_t>>>::entfilter()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(&local_collision::walkable_entity_table);
}
template <>
local_collision::entfilter<local_collision::entfilter_AND<local_collision::entfilter_ENTITY,
                                                          local_collision::entfilter_NO_CAPSULES>>::entfilter()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(&local_collision::entity_no_capsules_table);
}


template <>
local_collision::obbfilter<
    local_collision::obbfilter_AND<walkable_obbfilter_t, local_collision::obbfilter_OBB_LINE_SEGMENT_TEST>>::obbfilter()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(&local_collision::walkable_obb_table);
}

template <>
local_collision::entfilter<local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,
                                                          local_collision::entfilter_VALID_COLLISION_PAIR>>::entfilter()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(&local_collision::valid_pair_table);
}

template <>
local_collision::entfilter<
    local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,
                                   local_collision::entfilter_AND<local_collision::entfilter_VALID_COLLISION_PAIR,
                                                                  local_collision::entfilter_SPHERE_TEST>>>::entfilter()
{
    this->m_vtbl = reinterpret_cast<std::intptr_t>(&local_collision::valid_pair_sphere_table);
}


void local_collision_patch()
{
    REDIRECT(0x0052F009, find_intersection);
}
