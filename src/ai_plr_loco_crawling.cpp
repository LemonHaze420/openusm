#include "ai_plr_loco_crawling.h"

#include "ai_crawl_params_record.h"
#include "ai_std_hero.h"
#include "actor.h"
#include "collision_geometry.h"
#include "custom_math.h"
#include "func_wrapper.h"
#include "glass_house_manager.h"
#include "line_info.h"
#include "movement_info.h"
#include "oldmath_po.h"
#include "os_developer_options.h"
#include "physical_interface.h"
#include "trace.h"

double sub_48B6F0(const float *a1, bool a2)
{
    return (a2 ? a1[2] : a1[0]);
}

double sub_48B710(const float *a1, bool a2)
{
    return (a2 ? a1[3] : a1[1]);
}

bool sub_66024E(const line_info &a1)
{
    if (os_developer_options::instance->get_flag(mString{"FORCE_TIGHTCRAWL"})) {
        return true;
    }

    auto *hit_entity = a1.hit_entity.get_volatile_ptr();
    if (hit_entity == nullptr) {
        return false;
    }

    assert(hit_entity->is_an_actor());

    auto sub_66DFF7 = [](entity_base *ent) -> bool {
        return ent->is_ext_flagged(0x80000);
    };

    if (sub_66DFF7(hit_entity) && hit_entity->get_colgeom() != nullptr &&
        (hit_entity->get_colgeom()->get_type() != 1)) {
        return true;
    } else {
        return false;
    }
}

line_info *check_interior_transition(actor *owner, ai::crawl_params_record &record, ai::als_inode *animation,
                                     bool spidey, bool tight, bool force)
{
    if constexpr (!STANDALONE_SYSTEM) {
        line_info *(*func)(actor *, ai::crawl_params_record *, ai::als_inode *, bool, bool, bool) =
            CAST(func, 0x0047B1E0);
        return func(owner, &record, animation, spidey, tight, force);
    }
    record = {};
    static Var<line_info> surface{0x00959468};
    auto &hit = surface();
    hit.clear();
    if (!force &&
        (!owner->is_frame_delta_valid() || owner->get_movement_info()->field_0.get_position().length2() < EPSILON))
        return &hit;
    static constexpr float spidey_interior[] = {1.1f, 1.6f, 0.9f, 0.4f, 1.2f, 1.0f};
    static constexpr float venom_interior[] = {2.8f, 2.1f, 1.4f, 0.6f, 2.8f, 1.6f};
    const auto *distances = spidey ? spidey_interior : venom_interior;
    const auto position = owner->get_abs_position();
    const auto up = owner->get_abs_po().get_y_facing();
    const float reach = sub_48B6F0(distances, tight) * owner->get_render_scale().z;
    auto check = [](line_info &line) {
        return line.check_collision(
            *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
    };
    hit.field_0 = position;
    hit.field_C = position + owner->get_abs_po().get_z_facing() * reach;
    check(hit);
    if (!hit.collision) {
        hit.clear();
        return &hit;
    }
    bool floor_transition = false;
    if (hit.hit_norm.y > 0.732421875f) {
        const auto first_normal = hit.hit_norm;
        hit.clear();
        hit.field_0 = position;
        hit.field_C = position - YVEC * reach;
        check(hit);
        line_info overhead;
        overhead.field_0 = hit.hit_pos + hit.hit_norm * 0.025f;
        overhead.field_C = overhead.field_0 + YVEC * 2.0f;
        check(overhead);
        if (!overhead.collision) {
            floor_transition = true;
        } else if ((overhead.field_0 - overhead.hit_pos).length2() < 0.6400000453f) {
            owner->cancel_animated_movement(-first_normal, 0.0f);
            hit.clear();
            return &hit;
        }
    }
    if ((is_noncrawlable_surface(hit) && !(hit.hit_norm.y > 0.732421875f && floor_transition)) ||
        dot(up, hit.hit_norm) > 0.9848077893f || is_colinear(owner->get_abs_po().get_y_facing(), hit.hit_norm, 0.01f) ||
        !hit.collision) {
        hit.clear();
        return &hit;
    }
    bool use_tight = tight || sub_66024E(hit);
    bool probe_tight = use_tight;
    po destination;
    destination.set_po(owner->get_abs_po().get_y_facing(), hit.hit_norm, position);
    const po original_destination = destination;
    const float side = dot(hit.hit_norm, owner->get_abs_po().get_x_facing());
    if (side <= -0.5f || side >= 0.5f) {
        po rotation;
        rotation.set_rot(hit.hit_norm, (side <= 0.0f ? 1.0f : -1.0f) * 0.0872664675f);
        destination = sub_48F770(destination, rotation);
    }
    line_info clearance;
    auto wall_probe = [&](bool tight_probe, bool floor_probe) {
        clearance.clear();
        const float advance = floor_probe ? 0.5f : static_cast<float>(sub_48B710(distances, tight_probe));
        clearance.field_0 = position + destination.get_z_facing() * advance;
        clearance.field_C = clearance.field_0 - hit.hit_norm * distances[4];
        return check(clearance) && !is_noncrawlable_surface(clearance);
    };
    bool accepted = wall_probe(use_tight, floor_transition && hit.hit_norm.y > 0.732421875f);
    if (!accepted && !use_tight) {
        use_tight = true;
        accepted = wall_probe(true, false);
        probe_tight = true;
    }
    if (accepted) {
        clearance.field_0 = clearance.hit_pos + clearance.hit_norm * 0.1f;
        const float outward = floor_transition && hit.hit_norm.y > 0.732421875f ? 2.0f : 1.0f;
        clearance.field_C = clearance.field_0 + clearance.hit_norm * (outward - 0.1f);
        accepted = !check(clearance);
    }
    if (accepted) {
        const float offset = floor_transition && hit.hit_norm.y > 0.732421875f ? 1.0f : 0.5f;
        clearance.field_0 = hit.hit_pos + hit.hit_norm * offset;
        clearance.field_C = clearance.field_0 + original_destination.get_z_facing() *
                                                    (static_cast<float>(sub_48B710(distances, probe_tight)) + 0.1f);
        accepted = !check(clearance);
    }
    if (accepted && dot(hit.hit_norm, owner->get_abs_po().get_y_facing()) > -0.1736481935f &&
        glass_house_manager::is_point_in_glass_house(hit.hit_pos)) {
        record.field_0 = static_cast<crawl_transition_type_enum>(1);
        record.field_4 = use_tight;
        if (hit.hit_norm.y > 0.732421875f && floor_transition) {
            po transform;
            transform.set_po(-YVEC, owner->get_abs_po().get_y_facing(), owner->get_abs_position());
            entity_set_abs_po(owner, transform);
            owner->get_render_scale();
            record.field_5 = true;
            record.field_8 = 90.0f;
            record.field_C = 0.0f;
            record.field_10 = YVEC;
            record.field_1C = hit.hit_pos;
            record.field_28 = distances[5];
        } else {
            const float alignment = dot(hit.hit_norm, owner->get_abs_po().get_y_facing());
            const float lateral = dot(hit.hit_norm, owner->get_abs_po().get_x_facing());
            record.field_5 = false;
            record.field_8 = alignment >= 0.3826833963f ? 135.0f : 90.0f;
            record.field_C = lateral <= -0.5f ? 45.0f : lateral >= 0.5f ? -45.0f : 0.0f;
            record.field_10 = hit.hit_norm;
            record.field_1C = hit.hit_pos;
            record.field_28 = sub_48B6F0(distances, probe_tight) * owner->get_render_scale().z;
        }
        record.update_crawl_transition_als_params(animation);
    } else {
        owner->cancel_animated_movement(hit.hit_norm, 0.0f);
        hit.clear();
    }
    return &hit;
}

line_info *check_exterior_transition(actor *the_actor, ai::crawl_params_record &arg4, ai::als_inode *arg8, bool a4,
                                     bool a5, bool a6)
{
    TRACE("check_exterior_transition");

    if constexpr (1) {
        arg4.field_0 = static_cast<crawl_transition_type_enum>(0);
        arg4.field_4 = false;
        arg4.field_8 = 0.0;
        arg4.field_C = 0.0;
        arg4.field_10 = ZEROVEC;
        arg4.field_1C = ZEROVEC;
        arg4.field_28 = 0.0;
        arg4.field_5 = false;

        static Var<line_info> surface_info{0x009594C8};

        surface_info().clear();
        if (!a6) {
            if (!the_actor->is_frame_delta_valid()) {
                return &surface_info();
            }

            auto *move_info = the_actor->get_movement_info();
            const auto &pos = move_info->field_0.get_position();
            if (pos.length2() < 0.000099999997) {
                return &surface_info();
            }
        }

        static constexpr float spidey_ext[6] = {1.0, 1.2, 1.0, 0.40000001, 1.2, 1.0};
        static constexpr float venom_ext[6] = {2.0, 2.5, 1.0, 0.40000001, 2.0, 1.35};

        const float *v10 = (a4 ? spidey_ext : venom_ext);

        auto *v95 = v10;

        vector3d y_facing = the_actor->get_abs_po().get_y_facing();
        vector3d z_facing = the_actor->get_abs_po().get_z_facing();
        auto floor_offset = the_actor->physical_ifc()->get_floor_offset();

        surface_info().field_0 = the_actor->get_abs_position();

        auto v19 = the_actor->get_render_scale()[2];
        auto v20 = sub_48B6F0(v10, a5) * v19;
        surface_info().field_C = surface_info().field_0 + z_facing * v20;
        surface_info().check_collision(
            *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);

        if (surface_info().collision) {
            surface_info().clear();
            return &surface_info();
        }

        surface_info().clear();

        surface_info().field_C = the_actor->get_abs_position() - y_facing * (floor_offset + 0.1);

        auto v25 = the_actor->get_render_scale()[2];
        auto v26 = sub_48B6F0(v10, a5) * v25;
        surface_info().field_0 = surface_info().field_C + z_facing * v26;
        surface_info().field_C = surface_info().field_C - z_facing * v25;

        surface_info().check_collision(
            *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);

        auto line0_collision = [&]() -> bool {
            line_info line0{};
            line0.field_0 = surface_info().field_0 + y_facing * (floor_offset + 0.1);
            line0.field_C = surface_info().field_0 - y_facing * 0.1f;
            line0.check_collision(
                *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
            return line0.collision;
        }();

        auto dot_val = dot(the_actor->get_abs_po().get_y_facing(), surface_info().hit_norm);

        if (surface_info().collision && !line0_collision && dot_val < 0.92000002) {
            bool v91 = false;
            if (surface_info().hit_norm[1] > 0.73242188) {
                line_info line1{};
                line1.field_0 = surface_info().hit_pos + surface_info().hit_norm * 2;
                line1.field_C = YVEC * 2 + line1.field_0;
                line1.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
                if (line1.collision || is_colinear(YVEC, the_actor->get_abs_po().get_y_facing(), 0.0099999998)) {
                    auto a4 = line1.field_0 - line1.hit_pos;
                    if (a4.length2() < 0.64000005) {
                        the_actor->cancel_animated_movement(-surface_info().hit_norm, 0.0);
                        surface_info().clear();
                        return &surface_info();
                    }
                } else {
                    v91 = true;
                }
            }

            line_info v103{};
            bool cond0 = (a5 || sub_66024E(surface_info()));

            vector3d a2 = -the_actor->get_abs_po().get_y_facing();
            if (is_colinear(a2, surface_info().hit_norm, 0.0099999998)) {
                a2 = the_actor->get_abs_po().get_z_facing();
            }

            po v100;
            v100.set_po(a2, surface_info().hit_norm, the_actor->get_abs_position());
            po v99 = v100;

            po v102 = identity_matrix;
            if (surface_info().hit_norm[1] <= 0.73242188f || !v91) {
                auto v34 = dot(the_actor->get_abs_po().get_x_facing(), surface_info().hit_norm);
                if (v34 <= -1.5f || v34 >= 1.5f) {
                    auto v35 = (v34 >= 0.0 ? 1.0 : -1.0);
                    auto v81 = v35 * DEG_TO_RAD(5.0);
                    v102.set_rot(surface_info().hit_norm, v81);
                    v100 = sub_48F770(v100, v102);
                }
            }

            line_info line3{};

            auto *v38 = v95;
            line3.clear();
            auto v36 = v100.get_z_facing() * 0.5;
            line3.field_0 = surface_info().hit_norm * 0.2f + surface_info().hit_pos - v36;

            auto a3a = sub_48B710(v95, cond0);
            line3.field_C = line3.field_0 + v100.get_z_facing() * a3a;
            auto v40 = line3.check_collision(
                *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
            auto v93 = !v40;
            bool cond1 = v93;
            if (v40 && !cond0) {
                cond0 = true;
                line3.clear();
                auto v41 = v100.get_z_facing() * 0.5f;
                line3.field_0 = surface_info().hit_norm * 0.2f + surface_info().hit_pos - v41;
                auto a3b = sub_48B710(v38, cond0);
                line3.field_C = line3.field_0 + v100.get_z_facing() * a3b;
                v93 = !line3.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
            }

            if (!v93) {
                the_actor->cancel_animated_movement(-surface_info().hit_norm, 0.0);
                surface_info().clear();
                return &surface_info();
            }

            line3.clear();
            auto a4 = surface_info().hit_norm * 0.1f + surface_info().hit_pos;
            auto a3c = sub_48B710(v38, cond0);
            line3.field_0 = a4 + v100.get_z_facing() * a3c;
            line3.field_C = line3.field_0 - surface_info().hit_norm * 0.60000002;

            bool v47;
            if (!line3.check_collision(*local_collision::entfilter_entity_no_capsules,
                                       *local_collision::obbfilter_lineseg_test,
                                       nullptr) ||
                is_noncrawlable_surface(line3)) {
                v47 = false;
                if (!cond0) {
                    cond0 = true;
                    line3.clear();
                    auto a3d = sub_48B710(v38, true);
                    auto v48 = v100.get_z_facing() * a3d;
                    line3.field_0 = surface_info().hit_norm * 0.1 + surface_info().hit_pos + v48;
                    line3.field_C = line3.field_0 - surface_info().hit_norm * 0.30000001;
                    v47 = line3.check_collision(*local_collision::entfilter_entity_no_capsules,
                                                *local_collision::obbfilter_lineseg_test,
                                                nullptr) &&
                          !is_noncrawlable_surface(line3);
                }
            } else {
                v47 = true;
            }

            bool v88 = false;
            v93 = false;
            if (!v47 && surface_info().hit_norm[1] > 0.73242188) {
                cond0 = (a5 || sub_66024E(surface_info()));

                line3.clear();
                vector3d v52 = v100.get_z_facing() * 0.5;
                line3.field_0 = surface_info().hit_norm * 0.1 + surface_info().hit_pos + v52;
                line3.field_C = line3.field_0 - surface_info().hit_norm * 0.89999998;
                auto v55 = line3.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);

                v88 = !v55;
                if (v55) {
                    cond1 = (std::abs(line3.hit_pos[1] - surface_info().hit_pos[1]) <= 0.1);
                } else {
                    cond1 = true;
                    line3.field_C = line3.field_0 - surface_info().hit_norm * 1.1f;
                    v93 = !line3.check_collision(*local_collision::entfilter_entity_no_capsules,
                                                 *local_collision::obbfilter_lineseg_test,
                                                 nullptr);
                }
            }

            if (cond1) {
                if (v88) {
                    line3.field_0 = line3.field_C;
                    line3.field_C = line3.field_0 + v99.get_z_facing() * 0.69999999;
                } else {
                    line3.field_0 = line3.hit_pos + line3.hit_norm * 0.5f;
                    float v58 = (v91 && surface_info().hit_norm[1] > 0.73242188 ? 2.0 : 1.0);

                    auto a3e = v58 - 0.1;
                    line3.field_C = line3.field_0 + line3.hit_norm * a3e;
                }

                cond1 = !line3.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
            }

            if (cond1) {
                float v90;
                v90 = (v88 ? 0.1 : (v91 && surface_info().hit_norm[1] > 0.73242188) ? 1.0 : 0.5);

                line3.field_0 = surface_info().hit_pos + surface_info().hit_norm * v90;
                double v95 = (v88 ? 0.69999999 : sub_48B710(v38, cond0) + 0.1);

                line3.field_C = line3.field_0 + v99.get_z_facing() * v95;
                auto v61 = line3.check_collision(
                    *local_collision::entfilter_entity_no_capsules, *local_collision::obbfilter_lineseg_test, nullptr);
                cond1 = !v61;
                if (v61 && !cond0) {
                    cond0 = true;
                    v90 = (v88 ? 0.1 : (v91 && surface_info().hit_norm[1] > 0.73242188 ? 1.0 : 0.5));

                    line3.field_0 = surface_info().hit_pos + surface_info().hit_norm * v90;
                    double v95 = (v88 ? 1.5 : sub_48B710(v38, cond0) + 0.1);

                    line3.field_C = line3.field_0 + v100.get_z_facing() * v95;
                    cond1 = !line3.check_collision(*local_collision::entfilter_entity_no_capsules,
                                                   *local_collision::obbfilter_lineseg_test,
                                                   nullptr);
                }

                if (!cond1) {
                    the_actor->cancel_animated_movement(surface_info().hit_norm, 0.0);
                }
            }

            if (cond1) {
                cond1 = glass_house_manager::is_point_in_glass_house(surface_info().hit_pos);
            }

            if (cond1) {
                if (v91 && surface_info().hit_norm.y > 0.73242188) {
                    auto &a4 = the_actor->get_abs_position();
                    auto &v64 = the_actor->get_abs_po();

                    po v99;
                    v99.set_po(YVEC, v64.get_y_facing(), a4);
                    entity_set_abs_po(the_actor, v99);

                    auto v65 = the_actor->get_render_scale()[2];
                    auto v66 = (v88 ? v65 * 1.25 : v65 * v38[5]);


                    arg4.field_5 = true;

                    int v200 = (!v88 ? 2 : (v93 ? 4 : 3));

                    arg4.field_0 = static_cast<crawl_transition_type_enum>(v200);
                    arg4.field_4 = cond0;
                    arg4.field_8 = 90.0;
                    arg4.field_C = 0.0;
                    arg4.field_10 = YVEC;
                    arg4.field_1C = surface_info().hit_pos;
                    arg4.field_28 = v66;
                } else {
                    auto &v70 = the_actor->get_abs_po();
                    auto dot_value = dot(v70.get_y_facing(), surface_info().hit_norm);

                    auto v89 = (dot_value >= 0.3826834 ? 135.0 : 90.0);

                    auto v90 = 0.0;
                    auto &v71 = the_actor->get_abs_po();

                    auto v72 = dot(v71.get_x_facing(), surface_info().hit_norm);
                    if (v72 > -0.5f) {
                        if (v72 >= 0.5f) {
                            v90 = 45.0;
                        }
                    } else {
                        v90 = -45.0;
                    }

                    auto v95 = the_actor->get_render_scale()[2];
                    float v73 = (v88 ? v95 * 1.25 : sub_48B6F0(v38, cond0) * v95);

                    arg4.field_5 = false;
                    arg4.field_0 = static_cast<crawl_transition_type_enum>(2);
                    arg4.field_4 = cond0;
                    arg4.field_8 = v89;
                    arg4.field_C = v90;
                    arg4.field_10 = surface_info().hit_norm;
                    arg4.field_1C = surface_info().hit_pos;
                    arg4.field_28 = v73;
                }

                arg4.update_crawl_transition_als_params(arg8);

                return &surface_info();
            } else {
                the_actor->cancel_animated_movement(-surface_info().hit_norm, 0.0);
            }

        } else {
            surface_info().clear();
        }

        return &surface_info();

    } else {
        line_info *(*func)(actor *, ai::crawl_params_record *a2, ai::als_inode *a3, bool a4, bool a5, bool a6) =
            CAST(func, 0x0047BFF0);
        return func(the_actor, &arg4, arg8, a4, a5, a6);
    }
}
