#include "ai_path.h"

#include "actor.h"
#include "ai_quad_path_cell.h"
#include "ai_region_paths.h"
#include "collision_capsule.h"
#include "collision_geometry.h"
#include "common.h"
#include "entity_base.h"
#include "func_wrapper.h"
#include "physical_interface.h"
#include "region.h"
#include "subdivision_obb.h"
#include "terrain.h"
#include "trace.h"
#include "utility.h"
#include "vector3d.h"
#include "wds.h"

#include <cassert>
#include <stdarg.h>
#include <stdio.h>

VALIDATE_SIZE(ai_path, 0xA4);

static Var<_std::list<ai_path *>> dword_958164{0x00958164};

static _std::list<ai_path *> &all_ai_paths()
{
    if constexpr (STANDALONE_SYSTEM) {
        static _std::list<ai_path *> paths;
        return paths;
    } else {
        return dword_958164();
    }
}

ai_path::ai_path()
{
    this->field_0 = {};
    this->field_10 = {};
    this->field_20 = {};
    this->field_30 = {};
    this->field_88.field_0 = 0;
    this->field_A0 = false;

    this->field_88.field_0 = 0;
    ai_path::set_status(this, eAIPathStatus{0}, "OK");

    if constexpr (STANDALONE_SYSTEM) {
        all_ai_paths().push_back(this);
    } else {
        static Var<int *> dword_958168{0x00958168};
        auto *v2 = dword_958168();
        auto v3 = dword_958168()[1];
        auto a3 = reinterpret_cast<int>(this);
        int **(__fastcall * buy_node)(void *, void *, int, int, void *) = CAST(buy_node, 0x006B78D0);
        auto node = buy_node(&dword_958164(), nullptr, reinterpret_cast<int>(dword_958168()), v3, &a3);
        dword_958164()._Incsize(1u);
        v2[1] = reinterpret_cast<int>(node);
        *node[1] = reinterpret_cast<int>(node);
    }
    this->field_84 = nullptr;
}

void ai_path::frame_advance_all_ai_paths(Float)
{
    TRACE("ai_path::frame_advance_all_ai_paths");

    for (auto *path : all_ai_paths()) {
        if (path == nullptr || path->m_pathStatus.field_0 != 0) {
            continue;
        }

        bool unloaded_region = false;
        if (!path->field_10.empty()) {
            auto *last_region = reinterpret_cast<region *>(path->field_10.back());
            unloaded_region = last_region != nullptr && !last_region->is_loaded();
        }
        if (!unloaded_region && !path->field_0.empty()) {
            auto *last_region = path->field_0.back();
            unloaded_region = last_region != nullptr && !last_region->is_loaded();
        }
        if (unloaded_region) {
            set_status(path, eAIPathStatus{1}, "One or more of the necessary regions on our path is not loaded.");
        }
    }
}

ai_path::~ai_path()
{
    if constexpr (STANDALONE_SYSTEM) {
        auto &paths = all_ai_paths();
        for (auto it = paths.begin(); it != paths.end();) {
            if (*it == this)
                it = paths.erase(it);
            else
                ++it;
        }
    } else {
        void(__fastcall * remove)(void *, void *, void *) = CAST(remove, 0x005058F0);
        auto *self = this;
        remove(&dword_958164(), nullptr, &self);
    }

    this->field_30 = {};

    this->field_20 = {};

    this->field_10 = {};

    this->field_0 = {};
}

void ai_path::set_status(ai_path *a1, ai_path::eAIPathStatus a2, const char *Format, ...)
{
    char Dest[2048];

    va_list Args;

    va_start(Args, Format);
    a1->m_pathStatus = a2;
    vsprintf(Dest, Format, Args);
    auto *v3 = a1->field_88.get_volatile_ptr();
    if (v3 != nullptr) {
        mString v13{Dest};

        mString v11{": "};

        auto str = v3->field_10.to_string();
        mString v14{str};

        auto v16 = v14 + v11;

        auto v12 = v16 + v13;

        a1->field_90 = v12;

    } else {
        mString v15{Dest};

        a1->field_90 = v15;
    }

    va_end(Args);
}

ai_quad_path_cell *ai_path::advance_to_next_cell()
{
    if (!this->populate_quad_path_cell_route()) {
        return nullptr;
    }

    auto *v3 = this->field_30.m_first;

    auto result = this->field_30.back();
    if (v3 != nullptr) {
        if (this->field_30.m_last - v3) {
            --this->field_30.m_last;
        }
    }
    return result;
}

ai_quad_path_cell *ai_path::advance_to_farthest_direct_cell()
{
    return (ai_quad_path_cell *)THISCALL(0x0048A170, this);
}

bool ai_path::populate_quad_path_cell_route()
{
    return (bool)THISCALL(0x004899A0, this);
}

void ai_path::setup(entity_base_vhandle a2, const vector3d &a3, const vector3d &a4, bool a5, Float a6)
{
    if constexpr (1) {
        this->field_40 = a3;
        this->field_4C = a4;
        this->field_64[0] = this->field_40[0];
        auto v7 = this->field_40[1];
        auto v8 = this->field_40[2];
        this->field_64[1] = v7;
        this->field_64[2] = v8;
        this->field_58[0] = this->field_40[0];
        auto v9 = this->field_40[2];
        this->field_58[1] = this->field_40[1];
        this->field_58[2] = v9;
        this->field_A0 = a5;
        ai_path::set_status(this, eAIPathStatus{0}, "OK");

        this->field_0 = {};

        this->field_10 = {};

        this->field_20 = {};

        this->field_30 = {};

        auto v10 = a6 < 0.0f;
        auto v11 = equal(a6.value, 0.0f);

        this->field_84 = nullptr;
        this->field_88 = a2;

        float v12;
        if (v10 || v11) {
            v12 = 4.0f;
        } else {
            v12 = a6;
        }

        this->field_7C = v12;
        this->field_80 = 0.0;
        auto *v13 = bit_cast<actor *>(this->field_88.get_volatile_ptr());
        if (v13 != nullptr) {
            if (v13->is_an_actor()) {
                auto *v14 = v13->colgeom;
                if (v14 != nullptr) {
                    if (v14->get_type() == collision_geometry::CAPSULE) {
                        capsule v21 = bit_cast<collision_capsule *>(v14)->get_abs_capsule(v13->get_abs_po());
                        this->field_80 = v21.radius;
                    } else {
                        this->field_80 = v13->get_colgeom_radius();
                    }
                }
            }

            if (v13->has_physical_ifc() && a6 < 0.0f) {
                auto *v15 = v13->physical_ifc();
                auto v16 = v15->get_floor_offset() + 1.f;
                if (v16 > this->field_7C) {
                    this->field_7C = v16;
                }
            }
        }

        auto *v17 = ai_path::find_region_for_point(a3, this->field_7C);
        auto *v18 = ai_path::find_region_for_point(a4, this->field_7C);
        if (ai_path::find_region_route(v17, v18, &this->field_0)) {
            this->populate_quad_path_cell_route();
        } else if (v17 != nullptr) {
            if (v18 != nullptr) {
                auto &v19 = v18->get_name();
                auto &v20 = v17->get_name();
                ai_path::set_status(this, eAIPathStatus{1}, "Region pathfind failure between %s and %s", &v20, &v19);
            } else {
                ai_path::set_status(this, eAIPathStatus{1}, "Region pathfind failure (no end region)");
            }
        } else {
            ai_path::set_status(this, eAIPathStatus{1}, "Region pathfind failure (no start region)");
        }

    } else {
        THISCALL(0x00489C70, this, a2, &a3, &a4, a5, a6);
    }
}

bool ai_path::can_path_between_points(const vector3d &a1, const vector3d &a2, Float a6)
{
    vector3d v7;

    ai_path path{};

    path.setup(entity_base_vhandle{0}, a1, a2, false, a6);
    while ((!path.field_0.empty() || !path.field_20.empty() || !path.field_30.empty()) &&
           (path.m_pathStatus.field_0 == 0)) {
        v7 = path.get_next_point();
    }

    return (path.m_pathStatus.field_0 == 0);
}

vector3d ai_path::get_next_point()
{
    vector3d result;

    if constexpr (1) {
        if ((!this->field_0.empty() || !this->field_20.empty() || !this->field_30.empty()) &&
            this->m_pathStatus.field_0 == 0) {
            this->field_58 = this->field_64;

            ai_quad_path_cell *v9 = nullptr;
            if (this->field_A0) {
                v9 = this->advance_to_next_cell();
            } else {
                v9 = this->advance_to_farthest_direct_cell();
            }

            this->field_84 = v9;
            if (v9 != nullptr) {
                auto **v10 = this->field_30.m_first;
                if (v10 != nullptr && this->field_30.m_last - v10) {
                    this->field_64 = v9->get_edge_midpoint(*(this->field_30.m_last - 1));
                } else {
                    this->field_64 = this->field_70;
                }
            }

            result = this->field_64;

        } else {
            result = this->field_64;
        }

        return result;

    } else {
        THISCALL(0x0048A420, this, &result);
    }

    return result;
}

bool ai_path::find_closest_point_on_path_to_point(const vector3d &position, Float radius, vector3d *projected,
                                                  ai_quad_path **path, ai_quad_path_cell **cell)
{
    if (path)
        *path = nullptr;
    if (projected)
        *projected = position;
    auto *reg = find_region_for_point(position, radius);
    if (!reg)
        return false;
    auto *graph = reg->get_region_path_graph();
    if (!graph)
        return false;
    ai_quad_path_cell *found_cell = nullptr;
    auto *found_path = graph->get_quad_path_for_point(position, radius, &found_cell, true, nullptr);
    if (!found_cell)
        return false;
    if (path)
        *path = found_path;
    if (cell)
        *cell = found_cell;
    if (!found_cell->is_point_in_cell(position, radius.value))
        *projected = found_cell->closest_point(position);
    return true;
}

region *ai_path::find_region_for_point(const vector3d &a1, Float a2)
{
    if constexpr (1) {
        region *found_region = nullptr;

        _std::vector<region *> regions{};

        auto *the_terrain = g_world_ptr->the_terrain;
        the_terrain->find_regions(a1, &regions);

        for (auto &current_region : regions) {
            assert(current_region != nullptr);

            if (found_region != nullptr) {
                if (found_region->is_interior() || !current_region->is_interior()) {
                    if (found_region->is_interior() && current_region->is_interior()) {
                        auto v8 = current_region->obb->sub_52CA80();

                        if (found_region->obb->sub_52CA80() > v8) {
                            found_region = current_region;
                        }
                    }

                } else {
                    found_region = current_region;
                }

            } else {
                found_region = current_region;
            }
        }

        for (auto &current_region : regions) {
            assert(current_region != nullptr);

            if (current_region->has_quad_paths()) {
                auto *v12 = current_region->get_region_path_graph();
                if (v12 != nullptr) {
                    if (v12->get_quad_path_for_point(a1, a2, nullptr, false, nullptr) != nullptr) {
                        found_region = current_region;
                        break;
                    }
                }
            }
        }

        return found_region;

    } else {
        return (region *)CDECL_CALL(0x00479C50, &a1, a2);
    }
}

bool ai_path::find_region_route(region *a1, region *a2, _std::vector<region *> *route)
{
    assert(route->empty());

    if constexpr (1) {
        if (a1 == nullptr && a2 == nullptr) {
            return false;
        }

        if (a1 == a2) {
            void(__fastcall * sub_48F990)(void *, void *, void *) = CAST(sub_48F990, 0x0048F990);

            sub_48F990(route, nullptr, &a1);
            return true;
        }

        region::region_astar_search_record v6{};

        v6.setup(a1, a2);

        v6.search(0u);
        if (!v6.goal_found) {
            return false;
        }

        void(__fastcall * sub_48FE90)(void *, void *, void *) = CAST(sub_48FE90, 0x0048FE90);
        sub_48FE90(route, nullptr, &v6.field_24);

        return true;

    } else {
        return (bool)CDECL_CALL(0x00487F10, a1, a2, route);
    }
}

void ai_path_patch()
{
    REDIRECT(0x0055844B, ai_path::frame_advance_all_ai_paths);
}
