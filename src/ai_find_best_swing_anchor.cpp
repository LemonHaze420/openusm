#include "ai_find_best_swing_anchor.h"

#include "ai_state_swing.h"
#include "anchor_query_visitor.h"
#include "collide.h"
#include "common.h"
#include "conglomerate_clone.h"
#include "entity.h"
#include "func_wrapper.h"
#include "glass_house_manager.h"
#include "hierarchical_entity_proximity_map.h"
#include "line_info.h"
#include "loaded_regions_cache.h"
#include "local_collision.h"
#include "oldmath_po.h"
#include "physical_interface.h"
#include "quick_anchor_info.h"
#include "region.h"
#include "scratchpad_stack.h"
#include "stack_allocator.h"
#include "subdivision_obb.h"
#include "sweet_cone.h"
#include "utility.h"
#include "vector3d.h"
#include "vector4d.h"

#include <cassert>
#include <cmath>


#include <algorithm>
#include <cfloat>

VALIDATE_SIZE(occupancy_voxels_t, 0x1060u);

VALIDATE_SIZE(quick_anchor_container_t, 0x12C4u);

swing_anchor_finder::swing_anchor_finder(Float a1, Float a2, Float a3, Float a4, Float a5, Float a6)
{
    this->field_0 = a1;
    this->field_4 = a2;
    this->sweet_spot_distance = a3;
    this->field_C = a4;
    this->field_10 = a5;
    this->field_1C = a6;
    this->web_min_length = std::sqrt(a4);
    this->web_max_length = std::sqrt(a5);
    this->field_20 = std::sqrt(a6);
}

void swing_anchor_finder::set_max_pull_length(Float length)
{
    assert(length >= 0.0f && length < 30.0f && "Max pull length sanity check failed. Please inspect the callstack.");

    this->field_20 = length;
}

bool is_a_visible_swing_spot(const vector3d &a1, const vector3d &a2, local_collision::primitive_list_t **a1a,
                             local_collision::primitive_list_t ***a3)
{
    line_info line{};

    line.field_0 = a1;
    line.field_C = a2;

    auto local_vec3 = line.field_0 - line.field_C;
    local_vec3.normalize();

    local_vec3 *= LARGE_EPSILON;

    line.field_C += local_vec3;

    auto v11 = !local_collision::test_line_intersection_ex(a1a, line, a3);

    return v11;
}

bool swing_anchor_finder::accept_swing_point(const quick_anchor_info &info, const sweet_cone_t &sweet_cone,
                                             const Float &a4, Float a5, float *arg10,
                                             local_collision::primitive_list_t **a7,
                                             local_collision::primitive_list_t ***a8) const
{
    if constexpr (STANDALONE_SYSTEM) {
        vector3d normal = info.m_normal;

        assert(std::abs(normal.length() - 1.0f) < EPSILON);

        static auto live_in_glass_house_flag = glass_house_manager::is_enabled();

        bool result;

        if (!live_in_glass_house_flag || (result = glass_house_manager::is_point_in_glass_house(info.m_position))) {
            if (info.field_24 >= a4) {
                return false;
            }

            if (!sweet_cone.contains_point(info.m_position)) {
                return false;
            }

            auto v14 = info.m_position - sweet_cone.m_position;

            auto len2 = v14.length2();
            if (len2 <= this->field_C || len2 >= this->field_10 || a5 + this->web_min_length >= info.m_position[1]) {
                return false;
            }

            vector3d v29{info.m_position[0], a5, info.m_position[2]};

            auto *v27 = local_collision::obbfilter_lineseg_test;
            auto *v26 = local_collision::entfilter_reject_all;

            normal *= 0.2f;

            auto v25 = v29 + normal;

            auto v20 = info.m_position + normal;

            vector3d v5, a6;
            auto v21 = find_intersection(v20, v25, *v26, *v27, &v5, &a6, nullptr, nullptr, nullptr, false);
            auto v22 = info.m_position[1];

            double v23;
            bool v24;
            if (v21) {
                v23 = v22 - v5[1];
                v24 = v23 >= this->web_min_length;
            } else {
                v23 = v22 - a5;
                v24 = v23 > this->web_min_length;
            }

            if (v24 && (*arg10 = v23, std::sqrt(len2) - *arg10 <= this->field_20) &&
                is_a_visible_swing_spot(sweet_cone.m_position, info.m_position, a7, a8)) {
                result = true;
            } else {
                result = false;
            }
        }

        return result;
    } else {
        bool(__fastcall * func)(const swing_anchor_finder *,
                                void *edx,
                                const quick_anchor_info *,
                                const sweet_cone_t *,
                                const Float *,
                                Float a5,
                                float *,
                                local_collision::primitive_list_t **,
                                local_collision::primitive_list_t ***) = CAST(func, 0x00464590);
        return func(this, nullptr, &info, &sweet_cone, &a4, a5, arg10, a7, a8);
    }
}


static fixed_vector<local_collision::primitive_list_t, 7> &good_occluders()
{
    return var<fixed_vector<local_collision::primitive_list_t, 7>>(0x00958C38);
}

void sub_464850(local_collision::primitive_list_t **a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto **link = a1;
        int count = 0;
        while (*link != nullptr && ++count <= 1001) {
            auto *node = *link;
            const auto &cached = good_occluders();
            const bool preferred =
                link != a1 && std::any_of(cached.m_data, cached.m_data + cached.m_size, [node](const auto &entry) {
                    return entry.field_4.ent == node->field_4.ent;
                });
            if (preferred) {
                *link = node->field_0;
                node->field_0 = *a1;
                *a1 = node;
            } else {
                link = &node->field_0;
            }
        }
    } else {
        CDECL_CALL(0x00464850, a1);
    }
}

struct offset_anchor_filter : local_collision::obbfilter_base {
    vector3d field_4;
    float field_10;

    static bool __fastcall accept(const local_collision::obbfilter_base *base, void *, subdivision_node_obb_base *node,
                                  const local_collision::query_args_t *)
    {
        const auto *filter = static_cast<const offset_anchor_filter *>(base);
        return node->sphere_intersection(filter->field_4, filter->field_10);
    }

    offset_anchor_filter(const vector3d &a1, float a3)
    {
        if constexpr (STANDALONE_SYSTEM) {
            static const local_collision::obbfilter_base::native_vtable table{accept};
            this->m_vtbl = reinterpret_cast<std::intptr_t>(&table);
        } else {
            this->m_vtbl = 0x0087EDF8;
        }
        this->field_4 = a1;
        this->field_10 = a3;
    }
};

void occupancy_voxels_t::init(const vector3d &a2, const vector3d &a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        std::fill(std::begin(field_60), std::end(field_60), 0u);
        const vector4d first{a2.x, a2.y, a2.z, 1.0f};
        const vector4d second{a3.x, a3.y, a3.z, 1.0f};
        field_0 = vector4d::min(first, second);
        field_10 = vector4d::max(first, second);
        field_20 = field_10 - field_0;
        field_30 = {32.0f / field_20.x, 32.0f / field_20.y, 32.0f / field_20.z, 1.0f};
        field_40 = {31.0f, 31.0f, 31.0f, 1.0f};
        field_50 = {0.0f, 0.0f, 0.0f, 1.0f};
    } else {
        THISCALL(0x0048DCE0, this, &a2, &a3);
    }
}


bool swing_anchor_finder::find_best_offset_anchor(entity *self, const vector3d &a3, const vector3d &a4,
                                                  find_best_anchor_result_t *result) const
{
    if constexpr (!STANDALONE_SYSTEM) {
        bool(__fastcall * func)(const swing_anchor_finder *,
                                void *,
                                entity *,
                                const vector3d *,
                                const vector3d *,
                                find_best_anchor_result_t *) = CAST(func, 0x00486280);
        return func(this, nullptr, self, &a3, &a4, result);


    } else {
        if (!g_anchor_finding_enabled())
            return false;
        stack_allocator saved;
        scratchpad_stack::save_state(&saved);

        result->set_best_distance_squared(FLT_MAX);
        const auto position = self->get_abs_position();
        sweet_cone_t cone{this, position, self->get_abs_po().get_z_facing(), a3, a4};
        cone.field_24 = field_1C;
        const float radius = std::min((position - cone.sweet_spot).length(), 25.0f);
        float floor_y = cone.m_position.y - 15.0f;
        vector3d floor_point, floor_normal;
        if (find_intersection(cone.m_position,
                              cone.m_position - YVEC * 15.0f,
                              *local_collision::entfilter_entity_no_capsules,
                              *local_collision::obbfilter_lineseg_test,
                              &floor_point,
                              &floor_normal,
                              nullptr,
                              nullptr,
                              nullptr,
                              false))
            floor_y = floor_point.y;
        floor_y += 1.0f;

        const auto midpoint = (cone.sweet_spot + position) * 0.5f;
        const float half_distance = (cone.sweet_spot - position).length() * 0.5f;
        auto occluder_min = midpoint - vector3d{half_distance};
        const auto occluder_max = midpoint + vector3d{half_distance};
        occluder_min.y = position.y + 2.0f;
        local_collision::query_args_t args{};
        args.set_entity(self);
        auto *occluders = local_collision::query_line_segment(occluder_min,
                                                              occluder_max,
                                                              *local_collision::entfilter_blocks_ai_los,
                                                              *local_collision::obbfilter_accept_all,
                                                              args);
        sub_464850(&occluders);

        auto *grid = new (scratchpad_stack::alloc(sizeof(occupancy_voxels_t))) occupancy_voxels_t;
        grid->init(cone.sweet_spot - vector3d{25.0f}, cone.sweet_spot + vector3d{25.0f});
        ++subdivision_node_obb_base::visit_key();
        ++entity::visit_key3;
        offset_anchor_filter filter{cone.sweet_spot, radius};
        --subdivision_node_obb_base::visit_key();
        --entity::visit_key3;
        auto query_min = cone.sweet_spot - vector3d{radius};
        const auto query_max = cone.sweet_spot + vector3d{radius};
        query_min.y = position.y + 5.0f;
        auto *geometry = local_collision::query_line_segment(
            query_min, query_max, *local_collision::entfilter_reject_all, filter, args);
        auto *anchors = new (scratchpad_stack::alloc(sizeof(quick_anchor_container_t))) quick_anchor_container_t{};
        for (auto *node = geometry; node != nullptr; node = node->field_0) {
            fixed_vector<obb_closest_point_entry_t, 3> points;
            static_cast<subdivision_node_obb_base *>(node->get_obb_node())
                ->find_closest_point_on_visible_faces(cone.sweet_spot, position, &points);
            for (const auto &point : points) {
                auto anchor = point.field_0;
                if (point.field_C.y < 0.5f && point.field_C.y > -0.5f)
                    anchor += point.field_C * (5.0f / point.field_C.length());

                const auto delta = point.field_0 - cone.sweet_spot;
                auto direction = delta;
                if (direction.length2() > LARGE_EPSILON)
                    direction.normalize();
                const float score = delta.length2() * (2.0f - dot(cone.direction, direction));
                anchors->add_anchor(grid, anchor, point.field_C, point.field_0, score, nullptr, nullptr);
            }
        }
        ++entity::visit_key3;
        anchor_query_visitor visitor{anchors, cone.sweet_spot, position, true, grid};
        fixed_vector<region *, 15> regions;
        loaded_regions_cache::get_regions_intersecting_sphere(cone.sweet_spot, radius, &regions);
        for (auto *reg : regions)
            reg->visibility_map->traverse_sphere(cone.sweet_spot, radius, &visitor);
        anchors->field_0.sort();

        bool found = false;
        const auto accept = [&](const quick_anchor_info &anchor) {
            result->set_best_distance_squared(FLT_MAX);
            local_collision::primitive_list_t **occluder = nullptr;
            float target_length = -1.0f;
            const Float best_distance{result->get_best_distance_squared()};
            if (accept_swing_point(anchor, cone, best_distance, floor_y, &target_length, &occluders, &occluder)) {
                result->set_target_length(target_length);
                result->set_best_distance_squared(anchor.field_24);
                result->set_point(anchor.m_position);
                result->set_normal(anchor.m_normal);
                result->set_visual_point(anchor.field_18);
                result->set_entity(anchor.field_2C != nullptr ? nullptr : anchor.field_28);
                found = true;
            } else if (occluder != nullptr && occluders != *occluder) {
                auto *node = *occluder;
                *occluder = node->field_0;
                node->field_0 = occluders;
                occluders = node;
            }
        };
        const auto count = anchors->field_0.size();
        for (uint32_t index = 0; index < count && index < 20 && !found; ++index)
            accept(anchors->field_0.m_data[index]);
        for (uint32_t index = 20; index < count && !found; index += 2)
            accept(anchors->field_0.m_data[index]);
        scratchpad_stack::pop(anchors, sizeof(quick_anchor_container_t));
        local_collision::destroy_primitive_list(&geometry);
        auto &cached = good_occluders();
        cached.m_size = 0;
        for (auto *node = occluders; node != nullptr && cached.size() < 7; node = node->field_0)
            cached.push_back(*node);
        local_collision::destroy_primitive_list(&occluders);
        scratchpad_stack::restore_state(saved);
        return found;
    }
}

bool swing_anchor_finder::find_best_anchor(entity *a1, const vector3d &a2, find_best_anchor_result_t *a3) const
{
    return this->find_best_offset_anchor(a1, a2, ZEROVEC, a3);
}

void swing_anchor_finder::find_sweet_spot(entity *ent, const vector3d &a5, const vector3d &a6,
                                          vector3d *sweet_spot) const
{
    assert(sweet_spot != nullptr);

    assert(ent != nullptr);

    auto &abs_po = ent->get_abs_po();
    auto &z_facing = abs_po.get_z_facing();

    vector3d abs_pos = ent->get_abs_position();

    sweet_cone_t sweet_cone{this, abs_pos, z_facing, a5, a6};

    assert(sweet_cone.sweet_spot.is_valid());

    *sweet_spot = sweet_cone.sweet_spot;
}

void swing_anchor_finder::remove_all_anchors()
{
    ;
}

void quick_anchor_container_t::add_anchor(occupancy_voxels_t *grid, const vector3d &a3, const vector3d &a4,
                                          const vector3d &a5, Float a6, entity *a7, conglomerate_clone *a8)
{
    if constexpr (1) {
        vector4d a2a{a3[0], a3[1], a3[2], 1.f};

        if (grid == nullptr || !grid->is_occupied_if_not_occupy(a2a)) {
            quick_anchor_info anchor_info{};

            anchor_info.m_position = a3;

            anchor_info.m_normal = a4;

            anchor_info.field_18 = a5;

            anchor_info.field_24 = a6;
            anchor_info.field_28 = (entity_base *)a7;
            anchor_info.field_2C = a8;
            if (this->field_0.m_size >= 99) {
                this->field_0.sort();

                this->field_0.resize(50, quick_anchor_info{});
            }

            this->field_0.push_back(anchor_info);
        }
    }
}

bool occupancy_voxels_t::is_occupied_if_not_occupy(const vector4d &a2)
{
    if constexpr (1) {
        int minx, miny, minz, maxx, maxy, maxz;
        this->map_vector3d(a2, minx, miny, minz, maxx, maxy, maxz);

        uint32_t v3 = (1 << maxz) | (1 << minz);

        assert(minz >= 0 && maxz >= 0 && minz < MAX_3DGRID_BITS && maxz < MAX_3DGRID_BITS);

        for (auto x = minx; x <= maxx; ++x) {
            auto v6 = MAX_3DGRID_BITS * x;

            for (auto y = miny; y <= maxy; ++y) {
                assert(x >= 0 && y >= 0 && x < MAX_3DGRID_BITS && y < MAX_3DGRID_BITS);

                auto index = y + v6;

                assert(index >= 0 && index < (MAX_3DGRID_BITS * MAX_3DGRID_BITS));

                if ((v3 & this->field_60[index]) != 0) {
                    return true;
                }

                this->field_60[index] |= v3;
            }
        }

        return false;

    } else {
        return (bool)THISCALL(0x0048CEB0, this, a2);
    }
}

void occupancy_voxels_t::map_vector3d(const vector4d &a2, int &minx, int &miny, int &minz, int &maxx, int &maxy,
                                      int &maxz)
{
    if constexpr (1) {
        auto v8 = a2[0] - this->field_0[0];
        auto &max = this->field_40;
        auto v10 = a2[1] - this->field_0[1];
        auto &min = this->field_50;

        vector4d v27;
        v27[2] = a2[2] - this->field_0[2];
        v27[3] = a2[3] - this->field_0[3];

        vector4d v28;
        v28[0] = v8 * this->field_30[0];
        v27[0] = v28[0];
        v28[1] = v10 * this->field_30[1];
        v27[1] = v28[1];
        v28[2] = v27[2] * this->field_30[2];
        v27[2] = v28[2];
        v28[3] = v27[3] * this->field_30[3];
        v27[3] = v28[3];

        auto v12 = vector4d::floor(v27);
        v28 = vector4d::min(vector4d::max(v12, min), max);

        auto v14 = vector4d::ceil(v27);
        v27 = vector4d::min(vector4d::max(v14, min), max);

        minx = v28[0];

        miny = v28[1];

        minz = v28[2];

        maxx = v27[0];

        maxy = v27[1];

        maxz = v27[2];

        assert(minx >= 0 && minx < MAX_3DGRID_BITS);
        assert(miny >= 0 && miny < MAX_3DGRID_BITS);
        assert(minz >= 0 && minz < MAX_3DGRID_BITS);

    } else {
        THISCALL(0x0048CD70, this, &a2, &minx, &miny, &minz, &maxx, &maxy, &maxz);
    }
}

anchor_storage_class ai_find_best_pole(entity *self, const vector3d &a2, Float a3, Float a5, Float a6, Float a7)
{
    TRACE("ai_find_best_pole");

    if constexpr (STANDALONE_SYSTEM) {
        assert(self != nullptr);

        [[maybe_unused]] vector3d v71{};
        auto v70 = self->are_collisions_active();
        self->set_collisions_active(false, true);
        quick_anchor_container_t v69{};
        fixed_vector<region *, 15> v68{};
        auto abs_position = self->get_abs_position();
        auto a3a = abs_position;
        ++entity::visit_key3;
        anchor_query_visitor v66{&v69, a3a, a3a, false, nullptr};
        loaded_regions_cache::get_regions_intersecting_sphere(a3a, a3, &v68);
        for (uint32_t i = 0; i < v68.size(); ++i) {
            auto &v8 = v68.at(i);
            auto &v9 = v8->visibility_map;
            v9->traverse_sphere(a3a, a3, &v66);
        }

        v69.field_0.sort();
        entity_base_vhandle v27{0};
        vhandle_type<entity> v26{0};
        anchor_storage_class v64{v26, v27};

        auto func = [](anchor_storage_class *self, vhandle_type<entity> a2, entity_base_vhandle a3) -> void {
            self->field_0 = a2;
            self->field_4 = a3.field_0;
        };

        for (uint32_t j = 0; j < v69.field_0.size(); ++j) {
            auto &a2a = v69.field_0.at(j);
            auto v13 = a2a.m_position - a3a;
            auto v61 = v13.length2();
            if (v61 > a3 * a3) {
                break;
            }

            if (v69.field_0.at(j).field_28) {
                auto *v28 = v69.field_0.at(j).field_28;
                if (v28->get_flavor() == ANCHOR_MARKER) {
                    continue;
                }
            }

            auto *v60 = v69.field_0.at(j).field_28;
            entity_base_vhandle v34;
            if (v69.field_0.at(j).field_2C) {
                auto &v14 = v69.field_0.at(j);
                v34 = v14.field_2C->my_handle;
            } else {
                v34 = {0};
            }

            if (v60 != nullptr && v60->is_walkable()) {
                entity_base_vhandle v39;
                if (v69.field_0.at(j).field_2C) {
                    auto &v15 = v69.field_0.at(j);
                    v39 = v15.field_2C->my_handle;
                } else {
                    v39 = {0};
                }

                vhandle_type<entity> v40{v60->my_handle};
                func(&v64, v40, v39);
                vector3d v58{};
                if (v64.is_valid()) {
                    auto origin = v64.get_origin();
                    auto target = v64.get_target();
                    [[maybe_unused]] auto v57 = closest_point_segment(a3a, target, origin, v58);
                    line_info v56{};
                    v56.clear();
                    v56.field_0 = a3a;
                    v56.field_C = v58;
                    if (v56.check_collision(*local_collision::entfilter_entity_no_capsules,
                                            *local_collision::obbfilter_lineseg_test,
                                            nullptr)) {
                        entity_base_vhandle v27{0};
                        vhandle_type<entity> v26{0};
                        func(&v64, v26, v27);
                    }
                }

                if (v64.is_valid() && self->has_physical_ifc()) {
                    auto *v19 = self->physical_ifc();
                    auto v55 = v19->get_velocity();
                    auto v20 = v58 - a3a;
                    auto v54 = dot(v55, v20);
                    if (v54 >= 0.0 && a6 * a6 <= v55.length2()) {
                        if (a3a[1] <= v58[1]) {
                            if ((v58[1] - a3a[1]) > a5) {
                                entity_base_vhandle v27{0};
                                vhandle_type<entity> v26{0};
                                func(&v64, v26, v27);
                            }
                        } else if ((a3a[1] - v58[1]) > a5) {
                            entity_base_vhandle v27{0};
                            vhandle_type<entity> v26{0};
                            func(&v64, v26, v27);
                        }
                    } else {
                        entity_base_vhandle v27{0};
                        vhandle_type<entity> v26{0};
                        func(&v64, v26, v27);
                    }
                }

                if (v64.is_valid()) {
                    break;
                }
            }
        }

        self->set_collisions_active(v70, true);
        return v64;
    } else {
        anchor_storage_class result;
        CDECL_CALL(0x00486EE0, &result, self, &a2, a3, a5, a6, a7);

        return result;
    }
}


void swing_anchor_finder_patch()
{
    if constexpr (0) {
        {
            FUNC_ADDRESS(address, &swing_anchor_finder::accept_swing_point);
            //REDIRECT(0x00486BD5, address);
            //REDIRECT(0x00486D2A, address);
        }

        {
            FUNC_ADDRESS(address, &sweet_cone_t::contains_point);
            REDIRECT(0x0046463E, address);
        }

        REDIRECT(0x00464731, find_intersection);

        {
            FUNC_ADDRESS(address, &subdivision_node_obb_base::find_closest_point_on_visible_faces);
            REDIRECT(0x0048685C, address);
        }
    }
}
