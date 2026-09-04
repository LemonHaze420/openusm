#include "collide_trajectories.h"

#include "actor.h"
#include "collide_aux.h"
#include "collision_capsule.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "intersected_trajectory.h"
#include "intraframe_trajectory.h"
#include "line_segment.h"
#include "local_collision.h"
#include "pendulum.h"
#include "physical_interface.h"
#include "primitive_query_token.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include "trace.h"

#include <cmath>

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

    for (auto *iter = trj->field_168; iter != nullptr; iter = iter->field_4) {
        intersected_trajectory_t::pool().remove(iter);
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

        for (auto *v8 = v5; v8 != nullptr; v8 = v8->field_0) {
            local_collision::primitive_list_t::pool.remove(v8);
        }
    }
}

local_collision::primitive_list_t *query_potential_collision_primitives(const capsule &a1, const capsule &a2, actor *a3,
                                                                        intraframe_trajectory_t *a4)
{
    TRACE("query_potential_collision_primitives");

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
        if (i->ent != a3 && i->has_colgeom()) {
            auto v19 = i->get_bounding_sphere();

            auto v9 = [](sphere *self, const vector3d &a2, float a3) {
                auto v4 = self->center - a2;
                auto v7 = AbsSquared(v4);
                auto v6 = self->radius + a3;
                return (v6 * v6) >= v7;
            }(&v19, v18.center, v18.radius);

            if (v9) {
                dynamic_conglomerate_clone *v10 = nullptr;
                if (a4->ent->is_a_dynamic_conglomerate_clone()) {
                    v10 = CAST(v10, a4->ent);
                }

                static local_collision::entfilter<
                    local_collision::entfilter_AND<local_collision::entfilter_EXCLUDE_ENTITY,
                                                   local_collision::entfilter_VALID_COLLISION_PAIR>>
                    constraint_filter{};

                if (local_collision::collision_pair_matches_query_constraints(a4->ent, v10, constraint_filter, v20)) {
                    auto *mem = local_collision::primitive_list_t::pool.allocate_new_block();
                    v11 = new (mem) local_collision::primitive_list_t{i->ent, i->field_13C};
                    v11->field_10 = i;
                }
            }

            i->ent->field_64 = entity::visit_key3;
        }
    }

    --entity::visit_key3;
    local_collision::query_args_t v17{};
    auto *result =
        local_collision::query_sphere(v18.center, v18.radius, entf_36027, *local_collision::obbfilter_sphere_test, v17);

    local_collision::primitive_list_t *j = nullptr;
    for (j = (local_collision::primitive_list_t *)&v11; j->field_0 != nullptr; j = j->field_0) {
        ;
    }
    j->field_0 = result;

    return v11;
}

void resolve_rotations(intraframe_trajectory_t *a2, int a1)
{
    TRACE("resolve_rotations");

    if constexpr (1) {
        stack_allocator allocator;
        scratchpad_stack::save_state(&allocator);
        auto *token =
            static_cast<primitive_query_token_t *>(scratchpad_stack::alloc(sizeof(primitive_query_token_t) * a1));

        for (auto *trj = a2; trj != nullptr; trj = trj->field_15C) {
            if (a2->is_capsule) {
                assert(trj->world_po0.is_valid());

                assert(trj->world_po1.is_valid());

                token->field_0 = trj->get_abs_cap0();
                token->field_1C = trj->get_abs_cap1();

                auto v9 = token->field_1C.base - token->field_0.base;

                token->field_1C.base = token->field_1C.base - v9;
                token->field_1C.end = token->field_1C.end - v9;

                auto *v11 = query_potential_collision_primitives(token->field_0, token->field_1C, trj->ent, a2);
                token->field_38 = v11;
                sub_602E30(v11, a2);
                ++token;

            } else {
                token->field_38 = nullptr;
                ++token;
            }
        }

        for (auto *trj = a2; trj != nullptr; trj = trj->field_15C) {
            roll_back_rotation_and_rel_capsule_if_tunnelled(trj, token++);
        }

        scratchpad_stack::restore_state(allocator);

    } else {
        CDECL_CALL(0x00604A90, a2, a1);
    }
}

void resolve_collisions(intraframe_trajectory_t **a1, Float a2)
{
    CDECL_CALL(0x00604E30, a1, a2);
}

void resolve_moving_pendulums(intraframe_trajectory_t *a1, Float a2)
{
    if constexpr (1) {
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
    } else {
        CDECL_CALL(0x00605050, a1, a2);
    }
}
