#include "region.h"

#include "bitvector.h"
#include "common.h"
#include "conglom.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "hierarchical_entity_proximity_map.h"
#include "lego_map.h"
#include "light_source.h"
#include "memory.h"
#include "ngl.h"
#include "region_mash_info.h"
#include "subdivision_obb.h"
#include "terrain.h"
#include "texture_array.h"
#include "texture_to_frame_map.h"
#include "trace.h"
#include "wds.h"

#include <cassert>
#include <cfloat>
#include <cmath>

VALIDATE_SIZE(region, 0x134u);
VALIDATE_OFFSET(region, field_C4, 0xC4);
VALIDATE_OFFSET(region, field_108, 0x108);

static fixed_pool &lego_bitvector_pool = var<fixed_pool>(0x009222D4);

static constexpr auto REGION_UNINITIALIZED_STRIP_ID = -1;

static constexpr auto MAX_ALLOCATABLE_REGIONS = 256u;

region::region(const mString &a2)
{
    if constexpr (0) {
        this->visited = region::visit_key;
        this->field_58 = region::visit_key1;

        this->mash_info = new region_mash_info{};

        fixedstring<8> v1{a2.c_str()};
        this->mash_info->field_0 = v1.to_string();
        this->constructor_common();
    } else {
        THISCALL(0x0053B4B0, this, &a2);
    }
}

void *region::operator new(uint32_t)
{
    if (all_regions == nullptr) {
        all_regions = static_cast<region *>(arch_memalign(4u, sizeof(region) * MAX_ALLOCATABLE_REGIONS));
    }

    assert(number_of_allocated_regions < static_cast<int>(MAX_ALLOCATABLE_REGIONS));

    return &all_regions[number_of_allocated_regions++];
}

void region::constructor_common()
{
    this->bitvector_of_legos_rendered_last_frame = nullptr;
    this->field_98 = nullptr;
    this->obb = nullptr;
    this->vobbs_for_region_meshes = nullptr;
    this->field_38 = 0;
    this->field_3C = 0;
    this->m_fade_groups_count = 0;
    this->field_44 = 0;
    this->field_48 = 0;
    this->meshes = nullptr;
    this->texture_to_frame_maps = nullptr;
    this->m_total_frame_maps = 0;
    this->flags = 0;
    this->field_D4 = -1;
    this->multiblock_number = 1;
    this->district_id = 0;
    this->strip_id = -1;
    this->field_100 = nullptr;
    this->field_104 = nullptr;
    this->current_proximity_map_stack = nullptr;
    this->collision_proximity_map = nullptr;
    this->ai_proximity_map = nullptr;
    this->visibility_map = nullptr;
    this->light_proximity_map = nullptr;
    this->parking_proximity_map = nullptr;
    this->field_20 = 0;
    this->field_24 = 0;
    this->region_entities = nullptr;
    this->lights = nullptr;
}

static constexpr auto TEXTURES_LOADED = 0x40u;

void region::load_textures()
{
    assert((flags & TEXTURES_LOADED) == 0);

    for (auto i = 0; i < this->m_total_frame_maps; ++i) {
        auto &v3 = this->texture_to_frame_maps[i];
        auto *v8 = v3->get_ifl_name().c_str();
        tlFixedString v11{v8};
        auto Texture = nglGetTexture(v11);
        if (Texture != nullptr && Texture != nglDefaultTex && Texture != nglWhiteTex) {
            auto v5 = this->get_scene_id(true);

            auto v6 = v5.c_str();
            texture_array::load_map(v3, Texture, v6);
        }
    }

    this->flags |= TEXTURES_LOADED;
}

void region::unload_textures()
{
    if ((this->flags & TEXTURES_LOADED) != 0) {
        for (int i = 0; i < this->m_total_frame_maps; ++i) {
            auto &v3 = this->texture_to_frame_maps[i];
            auto *v5 = v3->get_ifl_name().c_str();
            tlFixedString a1{v5};
            auto *Texture = nglGetTexture(a1);
            if (Texture != nullptr && Texture != nglDefaultTex && Texture != nglWhiteTex) {
                texture_array::unload_map(v3, Texture);
            }
        }
        this->flags &= ~TEXTURES_LOADED;
    }
}

region::region_astar_search_record::region_astar_search_record()
{
    this->m_vtbl = 0x0087F0EC;
    this->field_24 = {};
}

void region::region_astar_search_record::setup(void *search_start, void *search_end)
{
    if constexpr (1) {
        auto *_search_start = static_cast<region *>(search_start);
        auto *_search_end = static_cast<region *>(search_end);

        assert(_search_start != nullptr);

        this->field_34 = _search_start->field_B0;

        if (_search_end != nullptr) {
            auto v3 = _search_end->field_B0 - this->field_34;

            this->field_40 = v3.length();
        } else {
            this->field_40 = 3.4028235e38;
        }

        astar_search_record::setup(_search_start, _search_end, &this->field_24, nullptr);

    } else {
        THISCALL(0x00519CB0, this, search_start, search_end);
    }
}

float region::get_ground_level() const
{
    if (this->obb != nullptr) {
        return this->field_BC;
    }

    return 0.0;
}

int region::get_strip_id() const
{
    assert(strip_id != REGION_UNINITIALIZED_STRIP_ID && "dsg and sin probably out of sync");

    return this->strip_id;
}

void region::set_strip_id(int a2)
{
    assert(strip_id == REGION_UNINITIALIZED_STRIP_ID);

    this->strip_id = a2;
}

void region::set_ambient(uint8_t a2, uint8_t a3, uint8_t a4)
{
    assert(mash_info != nullptr);

    this->mash_info->field_20 = color{a2 / 255.f, a3 / 255.f, a4 / 255.f, 1.0};
}

void region::remove(entity *a3)
{
    if constexpr (0) {
        assert(this->region_entities != nullptr);

        auto *v2 = a3;
        if (auto *v5 = bit_cast<light_source *>(a3); a3->is_a_light_source()) {
            this->remove(v5);
        } else {
            this->ai_proximity_map->remove_entity(v2);
            this->visibility_map->remove_entity(v2);
            this->parking_proximity_map->remove_entity(v2);
            if (auto *v3 = bit_cast<conglomerate *>(v2); v2->is_a_conglomerate()) {
                v3->remove_member_lights_from_region(this);
            }

            _std::list<entity *>::iterator ei;
            void(__fastcall * sub_506790)(void *, void *, _std::list<entity *>::iterator *out, entity **) =
                CAST(sub_506790, 0x00506790);
            sub_506790(this->region_entities, nullptr, &ei, &a3);

            //assert(ei != this->region_entities->end());

            void(__fastcall *
                 sub_56CAA0)(void *, void *, _std::list<entity *>::iterator *, _std::list<entity *>::iterator) =
                CAST(sub_56CAA0, 0x0056CAA0);
            sub_56CAA0(this->region_entities, nullptr, nullptr, ei);
        }
    } else {
        THISCALL(0x00545700, this, a3);
    }
}

void region::remove(light_source *a2)
{
    assert(a2 != nullptr);

    assert(this->lights != nullptr);

    this->light_proximity_map->remove_entity(a2);
    auto begin = this->lights->begin();
    auto end = this->lights->end();

    auto it = [](_std::vector<light_source *>::iterator it,
                 _std::vector<light_source *>::iterator end,
                 light_source *a4) -> _std::vector<light_source *>::iterator {
        for (; it != end; ++it) {
            if ((*it) == a4) {
                break;
            }
        }

        return it;
    }(begin, end, a2);

    std::for_each(it, end, [](light_source *&light) { light = nullptr; });
}

bool region::has_quad_paths() const
{
    return (this->flags & 0x800) != 0;
}

bool region::is_interior() const
{
    uint32_t v6 = this->flags;

    return ((v6 & 0x100) != 0) && ((v6 & 0x40000) != 0);
}

bool region::already_visited() const
{
    return visit_key == this->visited;
}


traffic_path_graph *region::get_traffic_path_graph()
{
    auto *result = g_world_ptr->the_terrain->traffic_ptr;
    if (result == nullptr) {
        result = this->field_100;
    }

    return result;
}

int region::get_district_id() const
{
    static constexpr auto REGION_UNINITIALIZED_DISTRICT_ID = 0;

    assert(district_id != REGION_UNINITIALIZED_DISTRICT_ID && "dsg and sin probably out of sync");

    return this->district_id;
}

bool region::is_loaded() const
{
    return (this->flags & 0x10) != 0;
}

void region::get_region_extents(vector3d *min_extent, vector3d *max_extent) const
{
    assert(min_extent != nullptr);

    assert(max_extent != nullptr);

    this->obb->get_extents(min_extent, max_extent);

    assert(not_equal(min_extent->x, FLT_MAX) && not_equal(min_extent->y, FLT_MAX) &&
           not_equal(min_extent->z, FLT_MAX) && not_equal(max_extent->x, -FLT_MAX) &&
           not_equal(max_extent->y, -FLT_MAX) && not_equal(max_extent->z, -FLT_MAX));
}

ai_region_paths *region::get_region_path_graph()
{
    if (this->is_loaded()) {
        return this->field_104;
    }

    return nullptr;
}

void region::sub_5452D0()
{
    if (0) {
#if 0
        auto *v2 = mem_alloc(0x10);

        if (v2) {
            v2->m_first = 0;
            v2->m_last = 0;
            v2->m_end = 0;
        } else {
            v2 = 0;
        }

        auto *v3 = this->mash_info;
        this->meshes = v2;
        //std::vector::reserve(v2, v3->field_30);

        auto *v4 = mem_alloc(0x10);
        this->lights = new (v4) _std::vector<light_source *>{};
        this->field_30 = (_STL::hashtable__ptr_entity *) stru_95F938.m_head->_Prev->_Myval;
        auto *v5 = stru_95F938.m_head->_Prev;
        if (v5 != stru_95F938.m_head) {
            v5->_Prev->_Next = v5->_Next;
            v5->_Next->_Prev = v5->_Prev;
            operator delete(v5);
            --stru_95F938.m_size;
        }

        this->create_proximity_maps();
#endif

    } else {
        THISCALL(0x005452D0, this);
    }
}

bool region::is_inside_or_on(const vector3d &a2) const
{
    return this->obb->point_inside_or_on(a2);
}

void region::create_proximity_maps()
{
    if constexpr (0) {
    } else {
        THISCALL(0x00544F60, this);
    }
}

const mString &region::get_scene_id(bool a2) const
{
    TRACE("region::get_scene_id");

    return (a2 && this->field_C4 > 0 ? this->field_88 : this->field_78);
}

void region::set_district_variant(int a2)
{
    auto *v3 = this->field_78.c_str();
    this->field_C4 = a2;
    mString a1{0, "%s_v%d", v3, a2};

    this->field_88 = a1;
}

fixedstring<8> &region::get_name()
{
    return this->mash_info->field_0;
}

void region::un_mash_lego_map(char *a2, int *a3)
{
    TRACE("region::un_mash_lego_map");

    if constexpr (1) {
        this->field_9C = (lego_map_root_node *)a2;
        this->field_9C->un_mash(a2, a3, this);
        auto *mem = lego_bitvector_pool.allocate_new_block();
        this->bitvector_of_legos_rendered_last_frame = new (mem) fixed_bitvector<uint, 2048>{};

        assert(bitvector_of_legos_rendered_last_frame != nullptr);
        auto sub_663403 = [](auto *self) -> void {
            for (auto i = 0u; i < 65u; ++i) {
                self->field_4[i] = -1;
            }
        };

        sub_663403(this->bitvector_of_legos_rendered_last_frame);
    } else {
        THISCALL(0x0054FEC0, this, a2, a3);
    }
}

void region::add(entity *e)
{
    TRACE("region::add");

    if constexpr (0) {
    } else {
        THISCALL(0x0054FF40, this, e);
    }
}

void region::add(light_source *l)
{
    THISCALL(0x00545780, this, l);
}

int region::get_district_variant() const
{
    auto *v1 = this->get_scene_id(false).c_str();
    string_hash v2{v1};

    auto *the_terrain = g_world_ptr->the_terrain;
    auto *pack_switch_info = the_terrain->field_24.get_pack_switch_info(v2);
    if (pack_switch_info != nullptr) {
        return pack_switch_info->field_8;
    } else {
        return this->field_C4;
    }
}

int region::get_num_neighbors() const
{
    return this->neighbors.size();
}

region *region::get_neighbor(int neighbor_index) const
{
    assert(neighbor_index >= 0);

    assert(static_cast<size_t>(neighbor_index) < this->neighbors.size());

    auto v4 = this->neighbors[neighbor_index];
    auto *the_terrain = g_world_ptr->get_the_terrain();
    return the_terrain->get_region(v4);
}

void region_patch()
{
    {
        void (region::*func)(entity *e) = &region::add;
        FUNC_ADDRESS(address, func);
        REDIRECT(0x004F537A, address);
        REDIRECT(0x0055ACD5, address);
    }
}
