#include "ai_find_best_swing_anchor.h"

#include "ai_state_swing.h"
#include "anchor_query_visitor.h"
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

void swing_anchor_finder::set_max_pull_length(Float length) {
    assert(length >= 0.0f && length < 30.0f &&
           "Max pull length sanity check failed. Please inspect the callstack.");

    this->field_20 = length;
}

bool is_a_visible_swing_spot(const vector3d &a1,
                             const vector3d &a2,
                             local_collision::primitive_list_t **a1a,
                             local_collision::primitive_list_t ***a3) {
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

bool swing_anchor_finder::accept_swing_point(const quick_anchor_info &info,
                                             const sweet_cone_t &sweet_cone,
                                             const Float &a4,
                                             Float a5,
                                             float *arg10,
                                             local_collision::primitive_list_t **a7,
                                             local_collision::primitive_list_t ***a8) const
{
    if constexpr (0)
    {
        vector3d normal = info.m_normal;

        assert(std::abs(normal.length() - 1.0f) < EPSILON);

        static auto live_in_glass_house_flag = glass_house_manager::is_enabled();

        bool result;

        if (!live_in_glass_house_flag ||
            (result = glass_house_manager::is_point_in_glass_house(info.m_position))) {
            if (info.field_24 >= a4) {
                return false;
            }

            if (!sweet_cone.contains_point(info.m_position)) {
                return false;
            }

            auto v14 = info.m_position - sweet_cone.m_position;

            auto len2 = v14.length2();
            if (len2 <= this->field_C || len2 >= this->field_10 ||
                a5 + this->web_min_length >= info.m_position[1]) {
                return false;
            }

            vector3d v29{info.m_position[0], a5, info.m_position[2]};

            auto *v27 = local_collision::obbfilter_lineseg_test;
            auto *v26 = local_collision::entfilter_reject_all;

            normal *= 0.2f;

            auto v25 = v29 + normal;

            auto v20 = info.m_position + normal;

            vector3d v5, a6;
            auto v21 =
                find_intersection(v20, v25, *v26, *v27, &v5, &a6, nullptr, nullptr, nullptr, false);
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
    }
    else
    {
        bool (__fastcall *func)(const swing_anchor_finder *, void *edx,
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

void sub_464850(local_collision::primitive_list_t **a1)
{
    CDECL_CALL(0x00464850, a1);
}

struct swing_anchor_obbfilter_t : local_collision::obbfilter_base {
    vector3d field_4;
    float field_10;

    swing_anchor_obbfilter_t(
            const vector3d &a1,
            float a3)
    {
        this->m_vtbl = 0x0087EDF8;
        this->field_4 = a1;
        this->field_10 = a3;
    }
};

void occupancy_voxels_t::init(const vector3d &a2, const vector3d &a3)
{
    if constexpr (0)
    {}
    else
    {
        THISCALL(0x0048DCE0, this, &a2, &a3);
    }
}

static Var<fixed_vector<local_collision::primitive_list_t, 7>> good_occluders_from_last_frame {0x00958C38};

bool swing_anchor_finder::find_best_offset_anchor(entity *self,
                                                  const vector3d &a3,
                                                  const vector3d &a4,
                                                  find_best_anchor_result_t *result) const
{
    if constexpr (0)
    {
        if ( !g_anchor_finding_enabled() ) {
            return false;
        }

        stack_allocator v95;
        scratchpad_stack::save_state(&v95);

        assert(self != nullptr);

        assert(self->is_an_actor());

        assert(result != nullptr);

        result->set_best_distance_squared(3.4028235e38);

        sweet_cone_t sweet_cone {
            this,
            self->get_abs_position(),
            self->get_abs_po().get_z_facing(),
            a3,
            a4};

        sweet_cone.field_24 = this->field_1C;

        vector3d v11 = self->get_abs_position();
        float v14 = (v11 - sweet_cone.sweet_spot).length();
        float arg4 = std::min(v14, 25.0f);

        float v72 = sweet_cone.m_position[1] - 15.0f;

        vector3d a5a {};
        vector3d a6 {};
        
        vector3d _a2a = sweet_cone.m_position - YVEC * 15.0f;
        if ( find_intersection(
                    sweet_cone.m_position,
                    _a2a,
                    *local_collision::entfilter_entity_no_capsules,
                    *local_collision::obbfilter_lineseg_test,
                    &a5a,
                    &a6,
                    nullptr,
                    nullptr,
                    nullptr,
                    false) )
        {
            v72 = a5a[1];
        }

        v72 += 1.0f;

        auto v17 = sweet_cone.sweet_spot + self->get_abs_position();
        auto v71 = v17 * 0.5f;

        auto v20 = sweet_cone.sweet_spot - self->get_abs_position();
        auto v23 = v20.length() * 0.5f;

        local_collision::query_args_t v90 {};
        v90.set_entity(self);

        static local_collision::entfilter<local_collision::entfilter_BLOCKS_AI_LOS> ent_ai_los_filter {};

        vector3d occluder_query_min = v71 - vector3d {v23};
        vector3d occluder_query_max = v71 + vector3d {v23};

        occluder_query_min[1] = self->get_abs_position()[1] + 2.0f;

        assert(occluder_query_max.y > occluder_query_min.y + 1.0f);

        auto v65 = v90;
        auto *a7 = local_collision::query_line_segment(occluder_query_min,
           occluder_query_max, ent_ai_los_filter, *local_collision::obbfilter_accept_all, v65);
        sub_464850(&a7);

        auto *mem = scratchpad_stack::alloc(4192);
        auto *v76 = new (mem) occupancy_voxels_t {};

        vector3d a2a = sweet_cone.sweet_spot + vector3d {25.0f};
        vector3d _v71 = sweet_cone.sweet_spot - vector3d {25.0f};

        v76->init(_v71, a2a);

        ++subdivision_node_obb_base::visit_key();
        ++entity::visit_key3;

        bool v66 = false;
        swing_anchor_obbfilter_t v92 {sweet_cone.sweet_spot, arg4};
        for ( int i = 0; i < 1; ++i )
        {
            --entity::visit_key3;
            --subdivision_node_obb_base::visit_key();

            vector3d min_query_extent = sweet_cone.sweet_spot - vector3d {arg4};

            vector3d max_query_extent = sweet_cone.sweet_spot + vector3d {arg4};

            min_query_extent[1] = sweet_cone.m_position[1] + 5.0f;

            assert(min_query_extent.y + 5.0f < max_query_extent.y && "Anchor query too thin. Please report to Andrei.");

            local_collision::query_args_t v65 {};
            auto *v75 = local_collision::query_line_segment(min_query_extent,
                    max_query_extent, *local_collision::entfilter_reject_all, v92, v65);
            fixed_vector<quick_anchor_info, 100> v98 {};

            auto *mem = scratchpad_stack::alloc(sizeof(quick_anchor_container_t));
            auto *v28 = new (mem) quick_anchor_container_t {};

            for ( auto *iter = v75; iter != nullptr; iter = iter->field_0 )
            {
                assert(!iter->is_entity());

                fixed_vector<obb_closest_point_entry_t, 3> v99;

                auto *obb_node = static_cast<subdivision_node_obb_base *>(iter->get_obb_node());
                obb_node->find_closest_point_on_visible_faces(
                        sweet_cone.sweet_spot,
                        self->get_abs_position(),
                        &v99);

                for ( auto &entry : v99 )
                {
                    vector3d v71 = entry.field_0;
                    auto v32 = entry.field_C[1];
                    if ( v32 < 0.5f && v32 > -0.5f )
                    {
                        auto v35 = entry.field_C * 5.0f;
                        auto v37 = entry.field_C.length();
                        vector3d a6 = v35 / v37;
                        v71 = entry.field_0 + a6;
                    }

                    auto v38 = (entry.field_0 - sweet_cone.sweet_spot).normalized();
                    auto v39 = (entry.field_0 - sweet_cone.sweet_spot).length2();

                    auto v107 = v39 * (2.0f - dot(sweet_cone.direction,v38));

                    v28->add_anchor(
                            v76,
                            v71,
                            entry.field_C,
                            entry.field_0,
                            v107,
                            nullptr,
                            nullptr);

                }
            }

            fixed_vector<region *, 15> v96 {};

            ++entity::visit_key3;

            anchor_query_visitor v90 {v28, sweet_cone.sweet_spot, sweet_cone.m_position, true, v76};

            auto v44 = arg4;

            loaded_regions_cache::get_regions_intersecting_sphere(sweet_cone.sweet_spot, arg4, &v96);

            for (auto &reg : v96)
            {
                reg->visibility_map->traverse_sphere(
                        sweet_cone.sweet_spot,
                        v44,
                        &v90);
            }

            v28->field_0.sort();

            for ( auto &anchor_info : v28->field_0 )
            {
                if ( v66 ) {
                    break;
                }

                result->set_best_distance_squared(3.4028235e38);

                local_collision::primitive_list_t **a8 = nullptr;
                local_collision::primitive_list_t *a7 = nullptr;
                float target_length = -1.0;

                auto best_distance_squared = result->get_best_distance_squared();

                if ( this->accept_swing_point(
                        anchor_info,
                        sweet_cone,
                        best_distance_squared,
                        v72,
                        &target_length,
                        &a7,
                        &a8) )
                {
                    assert(target_length > web_min_length);

                    result->set_target_length(target_length);

                    result->set_best_distance_squared(anchor_info.field_24);

                    result->set_point(anchor_info.m_position);
                    result->set_normal(anchor_info.m_normal);
                    result->set_visual_point(anchor_info.field_18);

                    result->set_entity(anchor_info.field_2C != nullptr ? nullptr : anchor_info.field_28);
                    v66 = true;
                }
                else if ( a8 != nullptr && a7 != *a8 )
                {
                    int count_before = 0;
                    bool in_list = false;
                    for ( auto *it = a7; it != nullptr; it = it->field_0 )
                    {
                        ++count_before;
                        if ( *a8 == it ) {
                            in_list = true;
                        }
                    }

                    assert(in_list);

                    auto *v52 = *a8;
                    *a8 = v52->field_0;
                    v52->field_0 = a7;
                    a7 = v52;

                    int count_after = 0;
                    for ( auto *it = v52; it != nullptr; it = it->field_0 ) {
                        ++count_after;
                    }

                    assert(count_after == count_before);
                }
            }

            for ( auto &anchor_info : v28->field_0 )
            {
                if ( v66 ) {
                    break;
                }

                result->set_best_distance_squared(3.4028235e38);

                auto best_distance_squared = result->get_best_distance_squared();

                local_collision::primitive_list_t **a8 = nullptr;
                local_collision::primitive_list_t *a7 = nullptr;
                float target_length = -1.0;
                if ( this->accept_swing_point(
                        anchor_info,
                        sweet_cone,
                        best_distance_squared,
                        v72,
                        &target_length,
                        &a7,
                        &a8) )
                {
                    assert(target_length >= web_min_length);

                    result->set_target_length(target_length);
                    result->set_best_distance_squared(anchor_info.field_24);
                    result->set_point(anchor_info.m_position);
                    result->set_normal(anchor_info.m_normal);
                    result->set_visual_point(anchor_info.field_18);


                    result->set_entity(anchor_info.field_2C != nullptr ? nullptr : anchor_info.field_28);
                    v66 = true;
                }
                else if ( a8 != nullptr && a7 != *a8 )
                {
                    int count_before = 0;
                    bool in_list = false;
                    for ( auto *jj = a7; jj != nullptr; jj = jj->field_0 )
                    {
                        ++count_before;
                        if ( *a8 == jj ) {
                            in_list = true;
                        }
                    }

                    assert(in_list);

                    auto *v61 = *a8;
                    *a8 = v61->field_0;
                    v61->field_0 = a7;
                    a7 = v61;

                    int count_after = 0;
                    for ( auto *kk = v61; kk != nullptr; kk = kk->field_0 ) {
                        ++count_after;
                    }

                    assert(count_after == count_before);
                }
            }

            if ( &v28->field_0 != &v98 ) {
                scratchpad_stack::pop(v28, 4804);
            }

            local_collision::destroy_primitive_list(&v75);
            if ( v66 ) {
                break;
            }
        }

        good_occluders_from_last_frame().m_size = 0;

        for ( auto *v62 = a7; v62 != nullptr && good_occluders_from_last_frame().size() != 7; v62 = v62->field_0 ) {
            good_occluders_from_last_frame().push_back(*v62);
        }

        local_collision::destroy_primitive_list(&a7);
        scratchpad_stack::restore_state(v95);
        return v66;
    }
    else
    {
        bool (__fastcall *func)(const swing_anchor_finder *, void *edx,
                              entity *self,
                              const vector3d *a3,
                              const vector3d *a4,
                              find_best_anchor_result_t *result) = CAST(func, 0x00486280);
        auto res = func(this, nullptr, self, &a3, &a4, result);

        if constexpr (0) {
            sp_log("");
            sp_log("%s, %s, %s",
                   result->m_point.to_string().c_str(),
                   result->m_normal.to_string().c_str(),
                   result->m_visual_point.to_string().c_str());
        }

        return res;
    }
}

bool swing_anchor_finder::find_best_anchor(entity *a1,
                                           const vector3d &a2,
                                           find_best_anchor_result_t *a3) const
{
    return this->find_best_offset_anchor(a1, a2, ZEROVEC, a3);
}

void swing_anchor_finder::find_sweet_spot(
        entity *ent,
        const vector3d &a5,
        const vector3d &a6,
        vector3d *sweet_spot) const
{
    assert(sweet_spot != nullptr);

    assert(ent != nullptr);

    auto &abs_po = ent->get_abs_po();
    auto &z_facing = abs_po.get_z_facing();

    vector3d abs_pos = ent->get_abs_position();

    sweet_cone_t sweet_cone {this, abs_pos, z_facing, a5, a6};

    assert(sweet_cone.sweet_spot.is_valid());

    *sweet_spot = sweet_cone.sweet_spot;
}

void swing_anchor_finder::remove_all_anchors() {
    ;
}

void quick_anchor_container_t::add_anchor(occupancy_voxels_t *grid,
                                        const vector3d &a3,
                                        const vector3d &a4,
                                        const vector3d &a5,
                                        Float a6,
                                        entity *a7,
                                        conglomerate_clone *a8)
{
    if constexpr (1)
    {
        vector4d a2a{a3[0], a3[1], a3[2], 1.f};

        if (grid == nullptr || !grid->is_occupied_if_not_occupy(a2a))
        {
            quick_anchor_info anchor_info{};

            anchor_info.m_position = a3;

            anchor_info.m_normal = a4;

            anchor_info.field_18 = a5;

            anchor_info.field_24 = a6;
            anchor_info.field_28 = (entity_base *) a7;
            anchor_info.field_2C = a8;
            if (this->field_0.m_size >= 99)
            {
                this->field_0.sort();

                this->field_0.resize(50, quick_anchor_info{});
            }

            this->field_0.push_back(anchor_info);

        }
        else
        {
            THISCALL(0x004900B0, this, grid, &a3, &a4, &a5, a6, a7, a8);
        }
    }
}

bool occupancy_voxels_t::is_occupied_if_not_occupy(const vector4d &a2)
{
    if constexpr (1)
    {
        int minx, miny, minz, maxx, maxy, maxz;
        this->map_vector3d(a2, minx, miny, minz, maxx, maxy, maxz);

        uint32_t v3 = (1 << maxz) | (1 << minz);

        assert(minz >= 0 && maxz >= 0 && minz < MAX_3DGRID_BITS && maxz < MAX_3DGRID_BITS);

        for (auto x = minx; x <= maxx; ++x)
        {
            auto v6 = MAX_3DGRID_BITS * x;

            for (auto y = miny; y <= maxy; ++y)
            {
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
        return (bool) THISCALL(0x0048CEB0, this, a2);
    }
}

void occupancy_voxels_t::map_vector3d(
    const vector4d &a2, int &minx, int &miny, int &minz, int &maxx, int &maxy, int &maxz)
{
    if constexpr (1)
    {
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

    }
    else
    {
        THISCALL(0x0048CD70, this, &a2, &minx, &miny, &minz, &maxx, &maxy, &maxz);
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
