#include "region.h"

#include "bitvector.h"
#include "common.h"
#include "conglom.h"
#include "dynamic_rtree.h"
#include "fixed_pool.h"
#include "func_wrapper.h"
#include "hierarchical_entity_proximity_map.h"
#include "proximity_map.h"
#include "lego_map.h"
#include "limbo_entities.h"
#include "light_source.h"
#include "loaded_regions_cache.h"
#include "memory.h"
#include "ngl.h"
#include "resource_pack_slot.h"
#include "region_mash_info.h"
#include "subdivision_obb.h"
#include "traffic_path_graph.h"
#include "terrain.h"
#include "texture_array.h"
#include "texture_to_frame_map.h"
#include "trace.h"
#include "wds.h"
#include "vtbl.h"

#include <cassert>
#include <cfloat>
#include <cmath>

VALIDATE_SIZE(region, 0x134u);
VALIDATE_OFFSET(region, field_C4, 0xC4);
VALIDATE_OFFSET(region, field_108, 0x108);

static fixed_pool &lego_bitvector_pool = var<fixed_pool>(0x009222D4);

#if STANDALONE_SYSTEM
namespace {
_std::list<_std::list<entity *> *> &entity_list_cache()
{
    static _std::list<_std::list<entity *> *> cache;
    static const bool initialized = [] {

        for (int i = 0; i < 9; ++i)
            cache.push_back(new _std::list<entity *>);
        return true;
    }();
    (void)initialized;
    return cache;
}
}
#endif

static constexpr auto REGION_UNINITIALIZED_STRIP_ID = -1;

static constexpr auto MAX_ALLOCATABLE_REGIONS = 256u;

region::region(const mString &a2)
{
    if constexpr (1) {
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

region::~region()
{
    destroy_proximity_maps();
    bitvector_of_legos_rendered_last_frame = nullptr;
    if ((flags & 8u) == 0 && mash_info != nullptr) {
        mash_info->~region_mash_info();
        mem_dealloc(mash_info, sizeof(region_mash_info));
        mash_info = nullptr;
    }
    if (obb != nullptr) {
        mem_dealloc(obb, sizeof(subdivision_node_obb_base));
        obb = nullptr;
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
    this->field_3C = nullptr;
    this->m_fade_groups_count = 0;
    this->field_44 = nullptr;
    this->field_48 = nullptr;
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
    assert(region_entities != nullptr);
    if (a3->is_a_light_source()) {
        remove(static_cast<light_source *>(a3));
        return;
    }
    ai_proximity_map->remove_entity(a3);
    visibility_map->remove_entity(a3);
    parking_proximity_map->remove_entity(a3);
    if (a3->is_flagged(4u))
        static_cast<conglomerate *>(a3)->remove_member_lights_from_region(this);
    auto &list = *static_cast<_std::list<entity *> *>(region_entities);
    auto found = std::find(list.begin(), list.end(), a3);
    assert(found != list.end());
    list.erase(found);
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

    if (it != end)
        *it = nullptr;
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
    auto *region_meshes = new _std::vector<nglMesh *>;
    region_meshes->reserve(mash_info->field_30);
    meshes = region_meshes;

    lights = new _std::vector<light_source *>;
#if STANDALONE_SYSTEM
    auto &cache = entity_list_cache();
    region_entities = cache.back();
    cache.pop_back();
#else
    region_entities = new _std::list<entity *>;
#endif

    create_proximity_maps();
}

void region::finish_unloading()
{
#if STANDALONE_SYSTEM
    if (bitvector_of_legos_rendered_last_frame != nullptr) {
        lego_bitvector_pool.remove(bitvector_of_legos_rendered_last_frame);
        bitvector_of_legos_rendered_last_frame = nullptr;
    }
    const auto detach = [this](entity *ent) {
        auto clear = reinterpret_cast<void(__fastcall *)(entity *, void *, region *, int)>(
            get_vfunc(ent->m_vtbl, 0x168));
        clear(ent, nullptr, this, static_cast<int>(0xDEADBEEFu));
    };
    auto *entities = static_cast<_std::list<entity *> *>(region_entities);
    while (!entities->empty())
        detach(entities->back());
    for (auto *light : *lights) {
        if (light != nullptr)
            detach(light);
    }
    delete lights;
    lights = nullptr;
    delete meshes;
    meshes = nullptr;
    entity_list_cache().push_back(entities);
    region_entities = nullptr;
    destroy_proximity_maps();
#else
    THISCALL(0x00545490, this);
#endif
}

void region::destroy_proximity_maps()
{
    collision_proximity_map = nullptr;
    const auto clear_map = [](hierarchical_entity_proximity_map *&map) {
        if (map == nullptr)
            return;
        for (int i = 0; i < map->number_of_levels; ++i) {
            map->maps[i]->initialized = false;
            map->maps[i] = nullptr;
        }
        map = nullptr;
    };
    clear_map(ai_proximity_map);
    clear_map(visibility_map);
    clear_map(light_proximity_map);
    clear_map(parking_proximity_map);
    if (current_proximity_map_stack != nullptr) {
        release_district_proximity_map_stack(current_proximity_map_stack);
        current_proximity_map_stack = nullptr;
    }
}

void region::set_loaded(bool loaded, resource_pack_slot *pack_slot)
{
    assert(pack_slot != nullptr);
    auto *region_meshes = meshes;
    assert(region_meshes != nullptr);

    if (!loaded) {
        unload_textures();
        finish_unloading();
        field_98 = nullptr;
        texture_to_frame_maps = nullptr;
        m_total_frame_maps = 0;
        if (field_100 != nullptr) {
            field_100->release_mem();
            field_100 = nullptr;
        }
        field_9C = nullptr;
        field_D4 = -1;
        flags &= ~0x10u;
        loaded_regions_cache::remove(this);
        return;
    }

    auto &directory = pack_slot->get_resource_directory();
    if (field_D4 == -1) {


        const mString mesh_names[] {
            get_scene_id(true) + mString {"R"},
            get_scene_id(true) + mString {"C"},
        };
        const string_hash mesh_hashes[] {
            string_hash {mesh_names[0].c_str()},
            string_hash {mesh_names[1].c_str()},
        };
        const int first = directory.get_type_start_idxs(RESOURCE_KEY_TYPE_MESH);
        const int end = first + directory.get_resource_count(RESOURCE_KEY_TYPE_MESH);
        for (int i = first; i < end; ++i) {
            auto *location = directory.get_resource_location(i);
            if (directory.get_mash_data(location->m_offset) != nullptr
                && (location->field_0.m_hash == mesh_hashes[0]
                    || location->field_0.m_hash == mesh_hashes[1])) {
                field_D4 = i;
                break;
            }
        }
    }
    if (field_D4 != -1) {
        auto *location = directory.get_resource_location(field_D4);
        tlFixedString mesh_name {};
        mesh_name.m_hash = location->field_0.m_hash.source_hash_code;
        for (auto *mesh = nglGetFirstMeshInFile(mesh_name);
             mesh != nullptr;
             mesh = nglGetNextMeshInFile(mesh)) {
            region_meshes->push_back(mesh);
        }
    }

    if ((flags & 0x40) == 0) {
        load_textures();
    }
    flags |= 0x10u;
    loaded_regions_cache::add(this);
    hash_update_bitvector().fill(0);
    last_update_slot() = 0;
    update_started() = true;
}

bool region::is_inside_or_on(const vector3d &a2) const
{
    return this->obb->point_inside_or_on(a2);
}

void region::create_proximity_maps()
{
#if STANDALONE_SYSTEM
    vector3d min_extent;
    vector3d max_extent;
    obb->get_extents(&min_extent, &max_extent);
    current_proximity_map_stack = acquire_district_proximity_map_stack();
    collision_proximity_map = &collision_dynamic_rtree();
    const auto create_map = [this]() {
        auto *map = static_cast<hierarchical_entity_proximity_map *>(
            current_proximity_map_stack->alloc(0x41C));
        if (map != nullptr) {
            new (map) hierarchical_entity_proximity_map;
            map->entity_data_lookup = {};
        }
        return map;
    };
    ai_proximity_map = create_map();
    visibility_map = create_map();
    light_proximity_map = create_map();
    parking_proximity_map = create_map();

    _std::vector<int> levels(2);
    levels[0] = 4;
    levels[1] = 16;
    visibility_map->init(*current_proximity_map_stack, 0, min_extent, max_extent, levels);
    ai_proximity_map->init(*current_proximity_map_stack, 2, min_extent, max_extent, levels);
    light_proximity_map->init(*current_proximity_map_stack, 3, min_extent, max_extent, levels);
    parking_proximity_map->init(*current_proximity_map_stack, 4, min_extent, max_extent, levels);

    field_9C = nullptr;
#else
    THISCALL(0x00544F60, this);
#endif
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

    this->field_9C = reinterpret_cast<lego_map_root_node *>(a2);
    this->field_9C->un_mash(a2, a3, this);
#if STANDALONE_SYSTEM
    if (!lego_bitvector_pool.m_initialized)
        lego_bitvector_pool.init(sizeof(fixed_bitvector<uint, 2048>), 8, 4, 0, 0, nullptr);
#endif
    auto *mem = lego_bitvector_pool.allocate_new_block();
    this->bitvector_of_legos_rendered_last_frame =
        new (mem) fixed_bitvector<uint, 2048> {};

    assert(bitvector_of_legos_rendered_last_frame != nullptr);
    for (auto i = 0u; i < 65u; ++i) {
        bitvector_of_legos_rendered_last_frame->field_4[i] = -1;
    }
}

void region::add(entity *e)
{
    TRACE("region::add");
#if STANDALONE_SYSTEM
    if (e == nullptr)
        return;
    if (e->is_a_light_source()) {
        this->add(static_cast<light_source *>(e));
        return;
    }
    auto *entities = static_cast<_std::list<entity *> *>(this->region_entities);
    assert(entities != nullptr);
    if (std::find(entities->begin(), entities->end(), e) == entities->end())
        entities->push_back(e);
    if (e->is_renderable() &&
        (e->is_flagged(0x200u) || e->is_flagged(4u) || e->is_a_conglomerate_clone()))
        visibility_map->update_entity(e);
    if (e->is_flagged(1u)) {
        auto update = reinterpret_cast<void(__fastcall *)(entity *, void *)>(
            get_vfunc(e->m_vtbl, 0x184));
        update(e, nullptr);
    }
    if (e->possibly_collide())
        collision_proximity_map->update_entity(e);
    if (e->is_a_parking_marker())
        parking_proximity_map->update_entity(e);
    if (e->is_flagged(4u))
        static_cast<conglomerate *>(e)->add_member_lights_to_region(this);
#else
    THISCALL(0x0054FF40, this, e);
#endif
}

void region::add(light_source *light)
{
#if STANDALONE_SYSTEM
    if (light == nullptr)
        return;
    assert(this->lights != nullptr);
    light_proximity_map->update_entity(light);
    for (auto *current : *this->lights) {
        if (current == light)
            return;
    }
    auto vacant = std::find(this->lights->begin(), this->lights->end(), nullptr);
    if (vacant != this->lights->end())
        *vacant = light;
    else
        this->lights->push_back(light);
#else
    THISCALL(0x00545780, this, light);
#endif
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
