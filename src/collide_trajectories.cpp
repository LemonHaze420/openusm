#include "collide_trajectories.h"

#include "actor.h"
#include "collide_aux.h"
#include "collision_capsule.h"
#include "collision_geometry.h"
#include "dirty_sphere.h"
#include "fixed_pool.h"
#include "intersected_trajectory.h"
#include "intraframe_trajectory.h"
#include "line_segment.h"
#include "local_collision.h"
#include "pendulum.h"
#include "physical_interface.h"
#include "primitive_query_token.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include "trajectory_cluster.h"

#include <cmath>
#include <algorithm>

static Var<fixed_pool> stru_937580{0x00937580};

sphere compute_bounding_sphere_for_trajectory_and_intersected_trajectories(intraframe_trajectory_t *trj)
{
    assert(trj != nullptr);

    auto v15 = trj->get_bounding_sphere();

    intraframe_trajectory_t *i = nullptr;
    for (auto *iter = trj->field_168; iter != nullptr; iter = iter->field_4) {
        assert(iter->trj != nullptr);

        if (iter->trj != i) {
            auto v16 = iter->trj->get_bounding_sphere();
            auto a2a = v15.radius + v16.radius;

            vector3d center;
            float radius;
            merge_spheres(v15.center, a2a, v16.center, a2a, center, radius);
            v15.center = center;
            v15.radius = radius;
        }

        i = iter->trj;
    }

    for (auto *iter = trj->field_168; iter != nullptr;) {
        auto *next = iter->field_4;
        intersected_trajectory_t::pool().remove(iter);
        iter = next;
    }

    trj->field_168 = nullptr;

    return v15;
}

void sub_602E30(local_collision::primitive_list_t *a1, intraframe_trajectory_t *trj)
{
    while (a1 != nullptr) {
        if (a1->is_entity()) {
            for (auto *i = trj; i != nullptr; i = i->field_15C) {
                if (a1->field_4.ent == i->ent && a1->field_8 == i->field_13C) {
                    a1->field_10 = i;
                    break;
                }
            }
        }

        a1 = a1->field_0;
    }
}

bool need_to_roll_back_rotation(const capsule &a3, const capsule &a4, local_collision::primitive_list_t *a1)
{
    auto v5 = (a3.end - a4.end);
    if (v5.length2() < 0.0000000099999991) {
        return false;
    }

    capsule a3a{a3.end, a4.end, LARGE_EPSILON};

    auto v18 = a3a.end - a3a.base;
    auto v17 = v18.length();
    if (v17 > EPSILON) {
        v18 /= v17;
    }

    auto v10 = a3a.end + v18 * LARGE_EPSILON;

    auto v9 = a3a.base - v18 * LARGE_EPSILON;

    line_segment_t lif{v9, v10};
    return local_collision::get_closest_line_intersection(a1, &lif, false, nullptr, nullptr, nullptr);
}

void roll_back_rotation_and_rel_capsule_if_tunnelled(intraframe_trajectory_t *trj, primitive_query_token_t *token)
{
    if (trj->is_capsule) {
        assert(trj->world_po0.is_valid());

        assert(trj->world_po1.is_valid());

        auto *v5 = token->field_38;
        if (need_to_roll_back_rotation(token->field_0, token->field_1C, v5)) {
            auto pos = trj->world_po1.get_position();

            trj->world_po1 = trj->world_po0;
            trj->world_po1.set_position(pos);

            if (trj->is_capsule) {
                trj->get_capsule().rel_cap = trj->my_abs_cap0;
            }

            trj->final_relcap = &trj->relcap0;
            trj->field_165 = true;
            trj->my_abs_cap1 = trj->my_abs_cap0;

            auto v12 = 1.f / trj->field_14C;

            vector3d v14;
            v14[0] = (trj->world_po1.m[3][0] - trj->world_po0.m[3][0]) * v12;
            v14[1] = (trj->world_po1.m[3][1] - trj->world_po0.m[3][1]) * v12;
            v14[2] = (trj->world_po1.m[3][2] - trj->world_po0.m[3][2]) * v12;
            trj->field_140 = v14;
        } else {
            auto pos = trj->world_po0.get_position();

            trj->world_po0 = trj->world_po1;

            trj->world_po0.set_position(pos);

            if (trj->is_capsule) {
                trj->get_capsule().rel_cap = trj->my_abs_cap1;
            }

            trj->final_relcap = &trj->relcap1;
            trj->my_abs_cap0 = trj->my_abs_cap1;
        }

        local_collision::destroy_primitive_list(&v5);
    }
}

local_collision::primitive_list_t *query_potential_collision_primitives(const capsule &a1, const capsule &a2, actor *a3,
                                                                        intraframe_trajectory_t *a4)
{
    local_collision::primitive_list_t *v11 = nullptr;

    sphere v18{};
    compute_bounding_sphere_for_two_capsules(a1, a2, &v18);
    ++entity::visit_key3;

    local_collision::query_args_t v20{};

    v20.field_10 = v18.center;
    v20.initialized_flags |= 0x3Cu;

    v20.field_28 = v18.radius;
    v20.field_2C = a3;
    v20.field_30 = a3;

    static local_collision::entfilter<
        local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,
                                       local_collision::entfilter_AND<local_collision::entfilter_VALID_COLLISION_PAIR,
                                                                      local_collision::entfilter_SPHERE_TEST>>>
        entf_36027{};

    for (auto *i = a4; i != nullptr; i = i->field_15C) {
        if (i->ent != a3 && i->ent->colgeom) {
            auto v19 = i->get_bounding_sphere();

            auto v9 = [](sphere *self, const vector3d &a2, float a3) {
                auto v4 = self->center - a2;
                auto v7 = AbsSquared(v4);
                auto v6 = self->radius + a3;
                return (v6 * v6) >= v7;
            }(&v19, v18.center, v18.radius);

            if (v9) {
                dynamic_conglomerate_clone *v10 = nullptr;
                if (i->ent->is_a_dynamic_conglomerate_clone()) {
                    v10 = CAST(v10, i->ent);
                }

                static local_collision::entfilter<
                    local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,
                                                   local_collision::entfilter_VALID_COLLISION_PAIR>>
                    constraint_filter{};

                if (local_collision::collision_pair_matches_query_constraints(i->ent, v10, constraint_filter, v20)) {
                    auto *mem = local_collision::primitive_list_t::pool.allocate_new_block();
                    auto *node = new (mem) local_collision::primitive_list_t{i->ent, i->field_13C};
                    node->field_0 = v11;
                    node->field_10 = i;
                    v11 = node;
                }
            }

            i->ent->field_64 = entity::visit_key3;
        }
    }

    --entity::visit_key3;
    auto *result =
        local_collision::query_sphere(v18.center, v18.radius, entf_36027, *local_collision::obbfilter_sphere_test, v20);

    local_collision::primitive_list_t *j = nullptr;
    for (j = (local_collision::primitive_list_t *)&v11; j->field_0 != nullptr; j = j->field_0) {
        ;
    }
    j->field_0 = result;

    return v11;
}

void resolve_rotations(intraframe_trajectory_t *trajectories, int count)
{
    stack_allocator allocator;
    scratchpad_stack::save_state(&allocator);
    auto *tokens = static_cast<primitive_query_token_t *>(
        scratchpad_stack::alloc(sizeof(primitive_query_token_t) * count));
    auto *token = tokens;
    for (auto *trj = trajectories; trj; trj = trj->field_15C, ++token) {
        token->field_38 = nullptr;
        if (!trj->is_capsule)
            continue;
        token->field_0 = trj->get_abs_cap0();
        token->field_1C = trj->get_abs_cap1();
        const auto delta = token->field_1C.base - token->field_0.base;
        token->field_1C.base -= delta;
        token->field_1C.end -= delta;
        token->field_38 = query_potential_collision_primitives(
            token->field_0, token->field_1C, trj->ent, trajectories);
        sub_602E30(token->field_38, trajectories);
    }
    token = tokens;
    for (auto *trj = trajectories; trj; trj = trj->field_15C)
        roll_back_rotation_and_rel_capsule_if_tunnelled(trj, token++);
    scratchpad_stack::restore_state(allocator);
}

namespace {
static Var<fixed_pool> contact_pool{0x0093755C};

void initialize_trajectory_pools()
{
#if STANDALONE_SYSTEM
    if (!contact_pool().m_base)
        contact_pool().init(28, 1024, 4, 1, 0, nullptr);
    if (!stru_937580().m_base)
        stru_937580().init(8, 128, 4, 1, 0, nullptr);
    if (!trajectory_cluster_t::pool().m_base)
        trajectory_cluster_t::pool().init(92, 16, 4, 1, 0, nullptr);
    if (!dirty_sphere_t::pool().m_base)
        dirty_sphere_t::pool().init(24, 128, 4, 1, 0, nullptr);
#endif
}

void set_trajectory_velocity(intraframe_trajectory_t *trj, const vector3d &velocity)
{
    trj->field_140 = velocity;
    trj->world_po1.set_position(trj->world_po0.get_position() + velocity * trj->field_14C);
}

void append_trajectories(intraframe_trajectory_t **head, intraframe_trajectory_t *list)
{
    while (*head)
        head = &(*head)->field_15C;
    *head = list;
}

bool overlaps(const sphere &first, const sphere &second, bool inclusive)
{
    const auto radius = first.radius + second.radius;
    const auto distance_squared = (first.center - second.center).length2();
    return inclusive ? distance_squared <= radius * radius : distance_squared < radius * radius;
}

dirty_sphere_t *add_dirty_sphere(intraframe_trajectory_t *trj, dirty_sphere_t *next)
{
    auto *dirty = new (dirty_sphere_t::pool().allocate_new_block()) dirty_sphere_t;
    dirty->bounds = compute_bounding_sphere_for_trajectory_and_intersected_trajectories(trj);
    dirty->next = next;
    dirty->trajectory = trj;
    return dirty;
}

trajectory_cluster_t *make_cluster(intraframe_trajectory_t *list, float remaining,
                                   float collision, int iteration)
{
    return new (trajectory_cluster_t::pool().allocate_new_block())
        trajectory_cluster_t(list, remaining, collision, iteration);
}

trajectory_cluster_t *recompute_clusters(trajectory_cluster_t *old,
                                         intraframe_trajectory_t **finished)
{
    trajectory_cluster_t *result = nullptr;
    auto **result_tail = &result;
    for (auto *cluster = old; cluster; cluster = cluster->next) {
        dirty_sphere_t *dirty = nullptr;
        for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C)
            if (trj->field_164)
                dirty = add_dirty_sphere(trj, dirty);
        auto **link = &cluster->trajectories;
        while (*link) {
            auto *trj = *link;
            if (!trj->field_164) {
                bool intersects_dirty = false;
                const auto bounds = trj->get_bounding_sphere();
                for (auto *sphere = dirty; sphere; sphere = sphere->next)
                    if (overlaps(bounds, sphere->bounds, false)) {
                        intersects_dirty = true;
                        break;
                    }
                if (intersects_dirty) {
                    trj->field_164 = true;
                    dirty = add_dirty_sphere(trj, dirty);
                } else {
                    *link = trj->field_15C;
                    trj->field_15C = *finished;
                    *finished = trj;
                    continue;
                }
            }
            link = &trj->field_15C;
        }
        trajectory_cluster_t *split = nullptr;
        while (dirty) {
            auto *item = dirty;
            dirty = dirty->next;
            trajectory_cluster_t *destination = nullptr;
            for (auto *candidate = split; candidate && !destination; candidate = candidate->next)
                for (auto *sphere = candidate->dirty_spheres; sphere; sphere = sphere->next)
                    if (overlaps(item->bounds, sphere->bounds, true)) {
                        destination = candidate;
                        break;
                    }
            if (!destination) {
                destination = make_cluster(item->trajectory, cluster->remaining_time,
                    cluster->collision_time, cluster->iteration);
                destination->next = split;
                split = destination;
            }
            item->next = destination->dirty_spheres;
            destination->dirty_spheres = item;
        }
        for (auto *piece = split; piece; piece = piece->next) {
            piece->trajectories = piece->dirty_spheres->trajectory;
            auto *sphere = piece->dirty_spheres;
            while (sphere) {
                auto *next = sphere->next;
                sphere->trajectory->field_15C = next ? next->trajectory : nullptr;
                dirty_sphere_t::pool().remove(sphere);
                sphere = next;
            }
            piece->dirty_spheres = nullptr;
        }
        *result_tail = split;
        while (*result_tail)
            result_tail = &(*result_tail)->next;
    }
    while (old) {
        auto *next = old->next;
        trajectory_cluster_t::pool().remove(old);
        old = next;
    }
    return result;
}

bool plane_intersection(const vector3d &start, const vector3d &end,
                        const vector3d &normal, float plane, vector3d &point)
{
    auto direction = end - start;
    const float length = direction.length();
    if (length > 0.0f)
        direction /= length;
    const float divisor = dot(direction, normal);
    if (divisor <= 0.0f && divisor >= 0.0f)
        return false;
    const float distance = (plane - dot(start, normal)) / divisor;
    point = start + direction * distance;
    return true;
}

void record_intersected_trajectory(intraframe_trajectory_t *trj,
                                  local_collision::primitive_list_t *primitive)
{
    if (!primitive || !primitive->field_10)
        return;
    auto *iterator = static_cast<intraframe_trajectory_t::iterator *>(
        stru_937580().allocate_new_block());
    iterator->trj = primitive->field_10;
    iterator->field_4 = trj->field_168;
    trj->field_168 = iterator;
}

void estimate_cluster_intersection(trajectory_cluster_t *cluster, primitive_query_token_t *tokens)
{
    cluster->collision_time = cluster->remaining_time;
    cluster->first_collision = nullptr;
    cluster->pair = {};
    cluster->tunnelled = false;
    auto *token = tokens;
    for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C, ++token) {
        trj->field_164 = false;
        if (!trj->is_capsule)
            continue;
        auto *intersections = local_collision::get_all_capsule_intersections(
            token->field_38, token->field_1C, trj->field_14C);
        for (auto *pair = intersections; pair; pair = pair->next) {
            trj->field_164 = trj->field_165 = true;
            vector3d other_velocity = ZEROVEC;
            if (pair->primitive && pair->primitive->field_10) {
                other_velocity = pair->primitive->field_10->field_140;
                record_intersected_trajectory(trj, pair->primitive);
            }
            const auto relative_velocity = trj->field_140 - other_velocity;
            const float speed = relative_velocity.length();
            float collision_time = 0.0f;
            if (speed > EPSILON) {
                const auto direction = relative_velocity / speed;
                const auto start = pair->point - relative_velocity * trj->field_14C;
                const auto other_start = pair->other_point - other_velocity * trj->field_14C;
                const auto end = pair->point + direction * token->field_1C.radius;
                vector3d contact;
                if (!plane_intersection(start, end, -direction, dot(-direction, other_start), contact)) {
                    collision_time = 3.402823466e38f;
                } else {
                    const float distance_squared = (contact - start).length2();
                    const float radius = token->field_1C.radius;
                    if (distance_squared >= radius * radius)
                        collision_time = (std::sqrt(distance_squared) - radius)
                            / (start - pair->point).length() * trj->field_14C;
                }
            }
            if (collision_time < cluster->collision_time) {
                cluster->collision_time = collision_time;
                cluster->first_collision = trj;
                cluster->pair = *pair;
            }
        }
        float swept_time = 3.402823466e38f;
        local_collision::closest_points_pair_t swept_pair{};
        if (swept_capsule_intersection(token->field_1C, token->field_0,
                cluster->remaining_time, token->field_0.radius, token->field_38,
                &swept_time, &swept_pair, &cluster->tunnelled, false)) {
            trj->field_164 = trj->field_165 = true;
            if (swept_time < cluster->collision_time) {
                record_intersected_trajectory(trj, swept_pair.primitive);
                cluster->collision_time = swept_time;
                cluster->first_collision = trj;
                cluster->pair = swept_pair;
            }
        }
        local_collision::destroy_primitive_list(&token->field_38);
        local_collision::destroy_closest_points_pair_list(&intersections);
    }
}

void estimate_first_intersections(trajectory_cluster_t *clusters)
{
    int count = 0;
    for (auto *cluster = clusters; cluster; cluster = cluster->next)
        for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C)
            ++count;
    stack_allocator allocator;
    scratchpad_stack::save_state(&allocator);
    auto *tokens = static_cast<primitive_query_token_t *>(
        scratchpad_stack::alloc(sizeof(primitive_query_token_t) * count));
    auto *token = tokens;
    for (auto *cluster = clusters; cluster; cluster = cluster->next)
        for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C, ++token)
            if (trj->is_capsule) {
                token->field_0 = trj->get_abs_cap0();
                token->field_1C = trj->get_abs_cap1();
                token->field_38 = query_potential_collision_primitives(
                    token->field_0, token->field_1C, trj->ent, cluster->trajectories);
                sub_602E30(token->field_38, cluster->trajectories);
            }
    token = tokens;
    for (auto *cluster = clusters; cluster; cluster = cluster->next) {
        estimate_cluster_intersection(cluster, token);
        for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C)
            ++token;
    }
    scratchpad_stack::restore_state(allocator);
}

bool is_fixed_for_collision(intraframe_trajectory_t *trj, entity *entity)
{
    if (!trj || trj->is_capsule) {
        if (entity && (entity->field_4 & 0x40u) != 0 && entity->has_physical_ifc()) {
            auto *physical = entity->physical_ifc();
            if ((physical->field_C & 0x80100u) == 0 && !physical->field_174)
                return false;
        }
    }
    return true;
}

void resolve_contact_velocities(intraframe_trajectory_t *trj,
    const local_collision::closest_points_pair_t &pair, bool &fixed)
{
    auto *other = pair.primitive ? pair.primitive->field_10 : nullptr;
    entity *other_entity = pair.primitive && pair.primitive->is_ent
        ? pair.primitive->field_4.ent : (other ? other->ent : nullptr);
    auto *entity = trj->ent;
    fixed = !trj->is_capsule || is_fixed_for_collision(trj, entity);
    bool other_fixed = is_fixed_for_collision(other, other_entity);
    const auto first_velocity = trj->field_140;
    const auto other_velocity = other ? other->field_140 : ZEROVEC;
    if (fixed && other_fixed) {
        if (entity->is_hero())
            fixed = false;
        else if (other_entity && other_entity->is_hero())
            other_fixed = false;
        else if (entity->get_colgeom() && entity->get_colgeom()->get_type() == 2)
            other_fixed = false;
        else if ((other_entity && other_entity->get_colgeom() &&
                  other_entity->get_colgeom()->get_type() == 2) || !other ||
                 other_velocity.length2() <= dot(first_velocity, other_velocity))
            fixed = false;
        else
            other_fixed = false;
    }
    const float first_mass = fixed ? 3.402823466e38f : 1.0f;
    const float second_mass = other_fixed ? 3.402823466e38f : 1.0f;
    const float first_inverse = fixed ? 0.0f : 1.0f;
    const float second_inverse = other_fixed ? 0.0f : 1.0f;
    const float impulse = -(dot(pair.normal, first_velocity - other_velocity) + LARGE_EPSILON)
        * 1.05f / pair.normal.length2() / (first_inverse + second_inverse);
    set_trajectory_velocity(trj, first_velocity + pair.normal * (impulse / first_mass));
    if (other)
        set_trajectory_velocity(other, other_velocity - pair.normal * (impulse / second_mass));
}

void resolve_first_collision(intraframe_trajectory_t *trj, intraframe_trajectory_t *cluster)
{
    if (!trj->is_capsule)
        return;
    const auto capsule = trj->get_abs_cap0();
    auto *primitives = query_potential_collision_primitives(capsule, capsule, trj->ent, cluster);
    sub_602E30(primitives, cluster);
    auto *pairs = local_collision::get_all_capsule_intersections(primitives, capsule, 0.0f);
    vector3d positive{-3.402823466e38f, -3.402823466e38f, -3.402823466e38f};
    vector3d negative{3.402823466e38f, 3.402823466e38f, 3.402823466e38f};
    float deepest = -3.402823466e38f;
    local_collision::closest_points_pair_t *deepest_pair = nullptr;
    for (auto *pair = pairs; pair; pair = pair->next) {
        bool fixed = false;
        resolve_contact_velocities(trj, *pair, fixed);
        auto *contact = static_cast<trajectory_contact_t *>(contact_pool().allocate_new_block());
        contact->normal = pair->normal;
        contact->other_velocity = pair->primitive && pair->primitive->field_10
            ? pair->primitive->field_10->field_140 : ZEROVEC;
        contact->next = trj->field_160;
        trj->field_160 = contact;
        const float penetration = capsule.radius - std::sqrt(pair->distance_squared);
        if (penetration > deepest) {
            deepest = penetration;
            deepest_pair = pair;
        }
        if (!fixed) {
            const auto offset = pair->normal * penetration;
            for (int axis = 0; axis != 3; ++axis) {
                if (offset[axis] > 0.0f) {
                    if (offset[axis] > positive[axis])
                        positive[axis] = offset[axis];
                } else if (offset[axis] < negative[axis]) {
                    negative[axis] = offset[axis];
                }
            }
        }
    }
    if (pairs)
        trj->field_165 = true;
    const auto original = trj->world_po0.get_position();
    auto position = original;
    for (int axis = 0; axis != 3; ++axis) {
        if (positive[axis] > 0.0f)
            position[axis] += positive[axis] + LARGE_EPSILON;
        if (negative[axis] < 0.0f)
            position[axis] += negative[axis] - LARGE_EPSILON;
    }
    trj->world_po0.set_position(position);
    trj->world_po1.set_position(trj->world_po1.get_position() + position - original);
    if (deepest_pair && trj->ent->has_physical_ifc()) {
        auto *physical = trj->ent->physical_ifc();
        physical->field_C |= 0x60u;
        physical->field_5C = deepest_pair->normal;
        physical->field_68 = deepest_pair->other_point;
    }
    local_collision::destroy_closest_points_pair_list(&pairs);
    local_collision::destroy_primitive_list(&primitives);
}

void shrink_velocity(vector3d &velocity, const vector3d &normal, const vector3d &other)
{
    if (normal.y < 0.0f) {
        const float own_horizontal = std::min(0.0f, velocity.x * normal.x + velocity.z * normal.z);
        const float other_horizontal = std::min(0.0f, other.x * normal.x + other.z * normal.z);
        const float horizontal = std::max(own_horizontal, own_horizontal - other_horizontal);
        if (horizontal <= 0.0f) {
            velocity.x -= horizontal * normal.x;
            velocity.z -= horizontal * normal.z;
        }
        const float own_vertical = std::min(0.0f, velocity.y * normal.y);
        const float other_vertical = std::min(0.0f, other.y * normal.y);
        const float vertical = std::max(own_vertical, own_vertical - other_vertical);
        if (vertical <= 0.0f)
            velocity.y -= vertical * normal.y;
    } else {
        const float own_dot = std::min(0.0f, dot(velocity, normal));
        const float other_dot = std::min(0.0f, dot(other, normal));
        const float correction = std::max(own_dot, own_dot - other_dot);
        if (correction <= 0.0f)
            velocity -= normal * correction;
    }
}
}

void resolve_collisions(intraframe_trajectory_t **trajectories, Float elapsed)
{
    initialize_trajectory_pools();
    intraframe_trajectory_t *without_geometry = nullptr;
    auto **link = trajectories;
    while (*link) {
        auto *trj = *link;
        if (trj->ent->get_colgeom()) {
            link = &trj->field_15C;
        } else {
            *link = trj->field_15C;
            trj->field_15C = without_geometry;
            without_geometry = trj;
        }
    }
    auto *active = make_cluster(*trajectories, elapsed.value, elapsed.value, 0);
    intraframe_trajectory_t *finished = nullptr;
    for (int iteration = 0; iteration != 4; ++iteration) {
        estimate_first_intersections(active);
        for (auto *cluster = active; cluster; cluster = cluster->next) {
            if (cluster->collision_time < 0.0f)
                cluster->collision_time = 0.0f;
            if (!cluster->first_collision)
                cluster->collision_time = cluster->remaining_time;
            cluster->collision_time += 0.0002500000118743628f;
            if (cluster->collision_time < LARGE_EPSILON && !cluster->tunnelled)
                cluster->collision_time = 0.009999999776482582f;
            if (cluster->collision_time > cluster->remaining_time)
                cluster->collision_time = cluster->remaining_time;
            cluster->remaining_time -= cluster->collision_time;
        }
        active = recompute_clusters(active, &finished);
        if (!active)
            break;
        for (auto *cluster = active; cluster; cluster = cluster->next) {
            for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C) {
                po integrated;
                trj->integrate(Float{cluster->collision_time}, &integrated);
                trj->field_14C = cluster->remaining_time;
                trj->world_po0 = integrated;
                trj->world_po1.set_position(integrated.get_position()
                    + trj->field_140 * cluster->remaining_time);
            }
            for (auto *trj = cluster->trajectories; trj; trj = trj->field_15C)
                resolve_first_collision(trj, cluster->trajectories);
            ++cluster->iteration;
        }
    }
    *trajectories = nullptr;
    while (active) {
        auto *next = active->next;
        for (auto *trj = active->trajectories; trj; trj = trj->field_15C)
            if (trj->field_14C > EPSILON) {
                trj->world_po1 = trj->world_po0;
                trj->field_14C = 0.0f;
                trj->field_140 = ZEROVEC;
            }
        append_trajectories(trajectories, active->trajectories);
        trajectory_cluster_t::pool().remove(active);
        active = next;
    }
    for (auto *trj = finished; trj; trj = trj->field_15C) {
        po integrated;
        trj->integrate(Float{trj->field_14C}, &integrated);
        trj->world_po0 = trj->world_po1 = integrated;
        trj->field_14C = 0.0f;
    }
    append_trajectories(trajectories, finished);
    append_trajectories(trajectories, without_geometry);
    for (auto *trj = *trajectories; trj; trj = trj->field_15C) {
        if (trj->ent->has_physical_ifc()) {
            auto *contact = trj->field_160;
            while (contact) {
                auto *next = contact->next;
                shrink_velocity(trj->field_150, contact->normal, contact->other_velocity);
                contact_pool().remove(contact);
                contact = next;
            }
            trj->field_160 = nullptr;
        }
        trj->backpropagate(elapsed);
    }
}

void resolve_moving_pendulums(intraframe_trajectory_t *a1, Float a2)
{
        intraframe_trajectory_t *a1a = nullptr;

        for (auto *v2 = a1; v2 != nullptr; v2 = v2->field_15C) {
            if (v2->ent->has_physical_ifc()) {
                auto *v3 = v2->ent->physical_ifc();
                if (v3->is_enabled()) {
                    if (v3->get_num_active_pendulums() > 0) {
                        for (auto j = 0; j < 5; ++j) {
                            auto *the_pendulum = v3->get_pendulum(j);
                            if (the_pendulum != nullptr && the_pendulum->has_a_moving_anchor()) {
                                auto v65 = v2->ent->get_abs_po();
                                auto v47 = v3->apply_positional_constraints(a2, v65.get_position(), false);
                                v65.set_position(v47);

                                auto *v9 = intraframe_trajectory_t::pool().allocate_new_block();
                                auto *v11 = new (v9) intraframe_trajectory_t{v2->ent, a2, v65, nullptr};

                                v11->field_15C = a1a;
                                a1a = v11;
                                v11->final_relcap = &v11->relcap0;
                                if (a2 > EPSILON) {
                                    auto *v18 = the_pendulum->get_volatile_ptr();
                                    auto v22 = v18->get_last_position();

                                    auto v23 = v18->get_abs_position() - v22;
                                    vector3d v51 = v23 / a2;

                                    auto pos = (v2->ent->field_A4 != 0
                                                    ? v2->ent->get_last_collision_free_state()->xform.get_position()
                                                    : v65.get_position());

                                    auto v34 = v2->ent->physical_ifc()->field_44;
                                    auto v63 = (v47 - pos) / a2;
                                    auto v60 = v63 + v34;
                                    v2->field_150 = v60 + v51;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (a1a != nullptr) {
            resolve_collisions(&a1a, a2);
        }

        intraframe_trajectory_t *v42 = nullptr;
        for (auto *k = a1a; k != nullptr; k = v42) {
            v42 = k->field_15C;
            intraframe_trajectory_t::pool().remove(k);
        }
}
