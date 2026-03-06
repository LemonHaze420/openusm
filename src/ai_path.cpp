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

ai_path::ai_path() {
    this->field_0 = {};
    this->field_10 = {};
    this->field_20 = {};
    this->field_30 = {};
    this->field_88.field_0 = 0;
    this->field_90 = {};
    this->field_A0 = false;

    this->field_88.field_0 = 0;
    ai_path::set_status(this, eAIPathStatus{0}, "OK");

    static Var<int *> dword_958168{0x00958168};

    auto *v2 = dword_958168();
    auto v3 = dword_958168()[1];
    auto a3 = (int) this;

    int ** (__fastcall *sub_6B78D0)(void *, void *, int, int, void *) = CAST(sub_6B78D0, 0x006B78D0);
    fastcall_call sub_48E040 = CAST(sub_48E040, 0x0048E040);

    auto v4 = sub_6B78D0(&dword_958164(), nullptr, (int) dword_958168(), v3, &a3);
    dword_958164()._Incsize(1u);
    v2[1] = (int) v4;
    *v4[1] = (int) v4;
    this->field_84 = nullptr;
}

void ai_path::frame_advance_all_ai_paths(Float a1) {
    TRACE("ai_path::frame_advance_all_ai_paths");

    CDECL_CALL(0x00479ED0, a1);
}

ai_path::~ai_path()
{
    [[maybe_unused]] int v4 = 3;

    void (__fastcall *sub_5058F0)(void *, void *, void *) = CAST(sub_5058F0, 0x005058F0);

    auto *v1 = this;

    sub_5058F0(&dword_958164(), nullptr, &v1);

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

ai_quad_path_cell *ai_path::advance_to_next_cell() {
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

ai_quad_path_cell *ai_path::advance_to_farthest_direct_cell() {
    return (ai_quad_path_cell *) THISCALL(0x0048A170, this);
}

bool ai_path::populate_quad_path_cell_route() {
    return (bool) THISCALL(0x004899A0, this);
}

void ai_path::setup(
    entity_base_vhandle a2, const vector3d &a3, const vector3d &a4, bool a5, Float a6) {
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
                        capsule v21 = bit_cast<collision_capsule *>(v14)->get_abs_capsule(
                            v13->get_abs_po());
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
                ai_path::set_status(this,
                                    eAIPathStatus{1},
                                    "Region pathfind failure between %s and %s",
                                    &v20,
                                    &v19);
            } else {
                ai_path::set_status(this,
                                    eAIPathStatus{1},
                                    "Region pathfind failure (no end region)");
            }
        } else {
            ai_path::set_status(this, eAIPathStatus{1}, "Region pathfind failure (no start region)");
        }

    } else {
        THISCALL(0x00489C70, this, a2, &a3, &a4, a5, a6);
    }
}

bool ai_path::can_path_between_points(const vector3d &a1, const vector3d &a2, Float a6) {
    vector3d v7;

    ai_path path{};

    path.setup(entity_base_vhandle{0}, a1, a2, false, a6);
    while ((!path.field_0.empty() || !path.field_20.empty() || !path.field_30.empty()) &&
           (path.m_pathStatus.field_0 == 0)) {
        v7 = path.get_next_point();
    }

    return (path.m_pathStatus.field_0 == 0);
}

vector3d ai_path::get_next_point() {
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

region *ai_path::find_region_for_point(const vector3d &a1, Float a2) {
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
        return (region *) CDECL_CALL(0x00479C50, &a1, a2);
    }
}

bool ai_path::find_region_route(region *a1, region *a2, _std::vector<region *> *route) {
    assert(route->empty());

    if constexpr (1) {

        if (a1 == nullptr && a2 == nullptr) {
            return false;
        }

        if (a1 == a2) {
            void (__fastcall *sub_48F990)(void *, void *, void *) = CAST(sub_48F990, 0x0048F990);

            sub_48F990(route, nullptr, &a1);
            return true;
        }

        region::region_astar_search_record v6{};

        v6.setup(a1, a2);

        v6.search(0u);
        if (!v6.goal_found) {
            return false;
        }

        void (__fastcall *sub_48FE90)(void *, void *, void *) = CAST(sub_48FE90, 0x0048FE90);
        sub_48FE90(route, nullptr, &v6.field_24);

        return true;

    } else {
        return (bool) CDECL_CALL(0x00487F10, a1, a2, route);
    }
}

void ai_path_patch() {
    REDIRECT(0x0055844B, ai_path::frame_advance_all_ai_paths);
}
