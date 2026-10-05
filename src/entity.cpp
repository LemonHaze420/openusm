#include "entity.h"
#include "aeps.h"

#include "collision_geometry.h"
#include "common.h"
#include "conglom.h"
#include "entity_mash.h"
#include "fixed_pool.h"
#include "fixed_vector.h"
#include "func_wrapper.h"
#include "hierarchical_entity_proximity_map.h"

#include "memory.h"
#include "moved_entities.h"
#include "region.h"
#include "terrain.h"
#include "time_interface.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "wds.h"
#include "dynamic_rtree.h"
#include "local_collision.h"

#include <cassert>

VALIDATE_SIZE(entity, 0x68u);

namespace entity_extended_regions_array_t {
#if STANDALONE_SYSTEM

static fixed_pool pool{sizeof(fixed_vector<region *, 7>), 32, 4, 1, 0, nullptr};
#else
static fixed_pool &pool = var<fixed_pool>(0x0091FF9C);
#endif
}

entity::entity(const string_hash &a2, uint32_t a3) : signaller(a2, a3, false)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[2];
#else
    m_vtbl = 0x00883F90;
#endif
    this->field_64 = 0;
    this->field_60 = 0;
    this->field_5C = 0;
    this->regions[0] = nullptr;
    this->regions[1] = nullptr;
    this->extended_regions = nullptr;
    this->field_58 = nullptr;
    this->colgeom = nullptr;
}

void entity::destroy_static_entity_pointers()
{
    CDECL_CALL(0x004D6940);
}

entity::~entity()
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[2];
#else
    m_vtbl = 0x00883F90;
#endif
    remove_from_regions();
    if (field_58 != nullptr) {
        auto *time = field_58;
        if (time->field_8) {
            auto destroy = reinterpret_cast<void(__fastcall *)(time_interface *, void *, bool)>(
                get_vfunc(time->m_vtbl, 0));
            destroy(time, nullptr, true);
        } else {
            auto release = reinterpret_cast<void(__fastcall *)(time_interface *, void *)>(
                get_vfunc(time->m_vtbl, 0x24));
            release(time, nullptr);
        }
        field_58 = nullptr;
    }
}

void entity::release_mem()
{
    remove_from_regions();
    if (field_58 != nullptr) {
        auto *time = field_58;
        if (time->field_8) {
            auto destroy = reinterpret_cast<void(__fastcall *)(time_interface *, void *, bool)>(
                get_vfunc(time->m_vtbl, 0));
            destroy(time, nullptr, true);
        } else {
            auto release = reinterpret_cast<void(__fastcall *)(time_interface *, void *)>(
                get_vfunc(time->m_vtbl, 0x24));
            release(time, nullptr);
        }
        field_58 = nullptr;
    }
    entity_base::release_mem();
}

void entity::randomize_position(const vector3d &a2, Float a3, Float a4, Float a5)
{
    TRACE("entity::randomize_position");

    THISCALL(0x004E2460, this, &a2, a3, a4, a5);
}

void entity::update_proximity_maps()
{
    TRACE("entity::update_proximity_maps");
    auto *root = this;
    if (is_flagged(0x8000u)) {
        root = static_cast<entity *>(get_conglom_owner());
    }
    if (root == nullptr || !root->is_renderable()) {
        return;
    }
    const bool should_update = root->is_flagged(0x200u);
    auto process_region = [root, should_update](region *current) {
        if (current == nullptr) {
            return;
        }
        if (should_update) {
            current->visibility_map->update_entity(root);
        } else {
            current->visibility_map->remove_entity(root);
        }
    };
    process_region(root->regions[0]);
    process_region(root->regions[1]);
    if (root->extended_regions != nullptr) {
        for (auto *current : *root->extended_regions) {
            process_region(current);
        }
    }
}

bool entity::is_in_limbo() const
{
    auto v1 = [](const entity *self) -> bool {
        return self->is_ext_flagged(0x2000u) || !bit_cast<entity *>(self)->get_primary_region();
    }(this);

    auto sub_6A7DAB = [](const entity_base *self) -> bool {
        return self->is_flagged(8u);
    };
    return v1 && !sub_6A7DAB(this);
}

void entity::_set_visible(bool visible, bool suppress_owner_update)
{
    if (is_flagged(0x200) == visible)
        return;
    field_4 = visible ? field_4 | 0x200 : field_4 & ~0x200u;
    if (!visible)
        set_occluded_last_frame(true);
    if (!is_flagged(0x8000)) {
        if (is_flagged(4)) {
            auto *self = static_cast<conglomerate *>(this);
            self->field_110 = (self->field_110 & ~0x11u) | 1;
        }
        update_proximity_maps();
    } else if (auto *owner = static_cast<conglomerate *>(get_conglom_owner())) {
        if (!suppress_owner_update)
            owner->update_proximity_maps();
        owner->field_110 = (owner->field_110 & ~0x11u) | 1;
    }
    if (!is_a_pfx_entity())
        aeps::DoCallback(this, 3, visible ? 0 : 0x10000000);
}

float entity::get_visual_radius()
{
    if constexpr (0) {
        if ((this->field_4 & 0x8004) == 0) {
            return 0.0f;
        }

        auto *parent = this->get_conglom_owner();
        if (parent == nullptr) {
            return 0.0f;
        }

        assert(parent->is_a_conglomerate());

        return parent->get_visual_radius();
    } else {
        auto *address = get_vfunc(m_vtbl, 0x28);
        if (address == nullptr) {
            return 0.0f;
        }
        float(__fastcall * func)(void *) = CAST(func, address);
        return func(this);
    }
}

vector3d entity::get_visual_center()
{
    vector3d result;
    if ((this->field_4 & 0x8004) != 0) {
        auto *parent = this->get_conglom_owner();

        if (parent != nullptr) {
            assert(parent->is_a_conglomerate());

            result = parent->get_visual_center();

        } else {
            result = this->get_abs_position();
        }

    } else {

        result = this->get_abs_position();
    }

    return result;
}

void entity::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("entity::un_mash");

    for (auto i = 0; i < 2; ++i) {
        this->regions[i] = nullptr;
    }

    this->extended_regions = nullptr;
    this->field_58 = nullptr;
    this->colgeom = nullptr;
    entity_base::_un_mash(a2, a3, a4);
}

void entity::clear_region(region *r, int i_know_what_i_am_doing)
{
    (void)i_know_what_i_am_doing;
    if (is_in_region(r))
        remove_me_from_region(r);
    if (regions[0] == nullptr)
        enter_limbo();
}

void entity::compute_sector(terrain *terrain_ptr, bool loading_scene, entity *fallback)
{
    auto compute = reinterpret_cast<void(__fastcall *)(entity *, void *, terrain *, bool, entity *)>(
        get_vfunc(m_vtbl, 0x16C));
    compute(this, nullptr, terrain_ptr, loading_scene, fallback);
}

void entity::_compute_sector(terrain *terrain_ptr, bool loading_scene, entity *fallback)
{

    (void)terrain_ptr;
    (void)loading_scene;
    (void)fallback;
    if (!is_flagged(0x10000000u))
        moved_entities::add_moved({get_my_handle()});
}

void entity::force_region_hack(region *a2)
{
    TRACE("entity::force_region_hack");

    assert(regions[0] == nullptr);

    this->regions[0] = a2;
}

void entity::force_region(region *r)
{
    TRACE("entity::force_region");

    if constexpr (0) {
        if (this->is_flagged(0x10000000)) {
            this->remove_from_regions();
        }

        this->set_flag_recursive(static_cast<entity_flag_t>(0x10000000u), true);
        if (r != nullptr) {
            if (!this->is_in_region(r)) {
                this->add_me_to_region(r);
            }
        }
    } else {
        void(__fastcall * func)(entity *, void *, region *) = CAST(func, get_vfunc(m_vtbl, 0x174));
        func(this, nullptr, r);
    }
}

void entity::force_current_region()
{
    this->set_flag_recursive(static_cast<entity_flag_t>(0x10000000u), true);
}

void entity::unforce_regions()
{
    if (this->is_flagged(0x10000000)) {
        this->remove_from_regions();
        this->set_flag_recursive(static_cast<entity_flag_t>(0x10000000), false);
    }
}

void entity::force_regions(entity *ent)
{
    field_4 |= 0x10000000u;
    region *visited_regions[15];
    int count = 0;
    for (int index = 0; ; ++index) {
        region *current = index < FIXED_REGIONS_ARRAY_SIZE ? ent->regions[index]
            : ent->extended_regions != nullptr &&
                      index - FIXED_REGIONS_ARRAY_SIZE < static_cast<int>(ent->extended_regions->size())
                ? ent->extended_regions->m_data[index - FIXED_REGIONS_ARRAY_SIZE] : nullptr;
        if (current == nullptr) {
            break;
        }
        visited_regions[count++] = current;
    }
    update_regions(visited_regions, count);
}

void entity::update_ai_proximity_map_recursive()
{

}

void entity::set_family_visible(bool a2)
{
    if constexpr (1) {
        if (!a2 || this->is_renderable()) {
            this->set_visible(a2, false);
        }

        for (auto *ent = this->get_first_child(); ent != nullptr; ent = ent->field_28) {
            if (ent->is_an_entity()) {
                bit_cast<entity *>(ent)->set_family_visible(a2);
            }
        }

    } else {
        THISCALL(0x004C07B0, this, a2);
    }
}

bool entity::is_renderable() const
{
    return this->is_flagged(0x100u);
}

bool entity::possibly_collide()
{
    return this->get_colgeom() != nullptr && this->are_collisions_active();
}

bool entity::possibly_camera_collide()
{
    auto *v1 = this->get_colgeom();
    return v1 != nullptr && this->are_collisions_active() && (v1->field_C & 0x10) != 0;
}

bool entity::possibly_walkable() const
{
    bool result = false;
    if (this->colgeom != nullptr) {
        if (this->are_collisions_active() && this->is_walkable()) {
            result = true;
        }
    }

    return result;
}

bool entity::sub_4C08E0()
{
    return this->colgeom != nullptr && this->are_collisions_active() && !this->is_ext_flagged(0x8000);
}

void entity::region_update_poss_collide()
{
    THISCALL(0x004C0810, this);
}

void entity::frame_advance(Float)
{
    ;
}

bool entity::is_still_visible()
{
    return (this->field_4 & 0x200) != 0;
}

void entity::render(Float a2)
{
    void(__fastcall * func)(entity *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x1AC));

    func(this, nullptr, a2);
}

nglMesh *entity::get_mesh()
{
    if constexpr (0) {
        return nullptr;
    } else {
        nglMesh *(__fastcall * func)(const entity *) = CAST(func, get_vfunc(m_vtbl, 0x1B0));

        return func(this);
    }
}

bool entity::has_mesh()
{
    return false;
}

void entity::suspend(bool propagate)
{
    auto callback = reinterpret_cast<void(__fastcall *)(entity *, void *, bool)>(get_vfunc(m_vtbl, 0x1B8));
    callback(this, nullptr, propagate);
}

void entity::unsuspend(bool propagate)
{
    auto callback = reinterpret_cast<void(__fastcall *)(entity *, void *, bool)>(get_vfunc(m_vtbl, 0x1BC));
    callback(this, nullptr, propagate);
}

void entity::set_render_color(color32 c)
{
    void(__fastcall * func)(entity *, void *, color32) = CAST(func, get_vfunc(m_vtbl, 0x1C0));
    func(this, nullptr, c);
}

color32 entity::get_render_color() const
{
    if constexpr (1) {
        color32(__fastcall * func)(const entity *) = CAST(func, get_vfunc(m_vtbl, 0x1C4));

        auto result = func(this);
        color32 col;
        std::memcpy(&col, &result, sizeof(result));
        return col;

    } else {
        color32 result;
        result.field_0[0] = -1;
        result.field_0[1] = -1;
        result.field_0[2] = -1;
        result.field_0[3] = -1;
        return result;
    }
}

void entity::set_render_alpha_mod(Float)
{
    ;
}

float entity::get_render_alpha_mod() const
{
    if constexpr (1) {
        float(__fastcall * func)(const entity *) = CAST(func, get_vfunc(m_vtbl, 0x1CC));

        return func(this);

    } else {
        return 0.0;
    }
}

void entity::set_render_scale(const vector3d &)
{
    ;
}

vector3d entity::get_render_scale() const
{
    vector3d result;
    result[0] = 1.0;
    result[1] = 1.0;
    result[2] = 1.0;

    return result;
}

void entity::set_render_zbias(Float)
{
    ;
}

float entity::get_render_zbias() const
{
    return 0.0;
}

light_manager *entity::get_light_set() const
{
    light_manager *(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x1E0));
    return func(this);
}

bool entity::can_be_a_lego()
{
    return false;
}

bool entity::can_be_a_conglom_clone()
{
    return false;
}

bool entity::is_a_dynamic_conglomerate_clone()
{
    return false;
}

void entity::set_collisions_active(bool a1, bool a2)
{
    void(__fastcall * func)(entity *, void *, bool, bool) = CAST(func, get_vfunc(m_vtbl, 0x1F0));

    func(this, nullptr, a1, a2);
}

bool entity::are_character_collisions_active()
{
    return false;
}

void entity::set_character_collisions_active(bool a2)
{
    if (a2) {
        this->field_4 |= 0x10;
    } else {
        this->field_4 &= 0xFFFFFFEF;
    }
}

void entity::set_terrain_collisions_active(bool a2)
{
    if (a2) {
        this->field_4 |= 0x20;
    } else {
        this->field_4 &= 0xFFFFFFDF;
    }
}

float entity::get_age()
{
    return 0.0;
}

void entity::set_age(Float)
{
    ;
}

void entity::set_recursive_age(Float a2)
{
    this->set_age(a2);
}

bool entity::is_in_region(const region *target) const
{
    if (target == nullptr) {
        return false;
    }
    for (auto *current : regions) {
        if (current == target) {
            return true;
        }
    }
    if (extended_regions != nullptr) {
        for (auto *current : *extended_regions) {
            if (current == target) {
                return true;
            }
        }
    }
    return false;
}

void entity::add_me_to_region(region *r)
{
    TRACE("entity::add_me_to_region");

    if constexpr (1) {
        assert((!is_a_conglomerate() || !((conglomerate *)this)->is_cloned_conglomerate()) &&
               "A cloned conglomerate should NEVER be put into a region!!!");

        assert(r != nullptr);

        auto ent_to_add = this->get_my_vhandle();
        assert(ent_to_add != INVALID_HANDLE);

        if (!this->is_in_region(r)) {
            if (r->is_loaded()) {
                int i = 0;
                for (; i < 2; ++i) {
                    if (this->regions[i] == nullptr) {
                        this->regions[i] = r;
                        break;
                    }
                }

                if (i == 2) {
                    if (this->extended_regions == nullptr) {
                        auto *mem = entity_extended_regions_array_t::pool.allocate_new_block();
                        auto *v5 = new (mem) fixed_vector<region *, 7>{};
                        this->extended_regions = v5;
                    }

                    assert(extended_regions != nullptr);

                    this->extended_regions->push_back(r);
                }

                r->add(this);
            } else {
                this->enter_limbo();
            }
        }
    } else {
        THISCALL(0x004F52C0, this, r);
    }
}

collision_geometry *entity::get_colgeom() const
{
    return this->colgeom;
}
float entity::get_colgeom_radius() const
{
    auto callback = reinterpret_cast<float(__fastcall *)(const entity *, void *)>(get_vfunc(m_vtbl, 0x254));
    return callback(this, nullptr);
}
vector3d entity::get_colgeom_center() const
{
    auto callback = reinterpret_cast<vector3d *(__fastcall *)(const entity *, void *, vector3d *)>(
        get_vfunc(m_vtbl, 0x258));
    vector3d result;
    callback(this, nullptr, &result);
    return result;
}

void entity::remove_me_from_region(region *target)
{
    assert(target != nullptr);
    if (target->is_loaded()) {
        target->remove(this);
    }

    region *remaining[9]{};
    int count = 0;
    for (auto *current : regions) {
        if (current != nullptr && current != target) {
            remaining[count++] = current;
        }
    }
    if (extended_regions != nullptr) {
        for (auto *current : *extended_regions) {
            if (current != nullptr && current != target) {
                remaining[count++] = current;
            }
        }
    }

    regions[0] = count > 0 ? remaining[0] : nullptr;
    regions[1] = count > 1 ? remaining[1] : nullptr;
    if (extended_regions != nullptr) {
        extended_regions->m_size = 0;
        for (int index = 2; index < count; ++index) {
            extended_regions->push_back(remaining[index]);
        }
        if (extended_regions->size() == 0) {
            entity_extended_regions_array_t::pool.remove(extended_regions);
            extended_regions = nullptr;
        }
    }
    if (regions[0] == nullptr) {
        enter_limbo();
    }
}

void entity::create_time_ifc()
{
    if constexpr (1) {
        auto *mem = mem_alloc(sizeof(time_interface));

        this->field_58 = new (mem) time_interface{this};
    } else {
        THISCALL(0x004DBA10, this);
    }
}

bool entity::is_indoors()
{
    return (bool)THISCALL(0x004CB670, this);
}

void entity::update_regions(region **visited_regions, int a3)
{
    if constexpr (1) {
        if ((a3 != 1 || visited_regions[0] != this->regions[0] || this->regions[1] != nullptr) &&
            (a3 != 2 || visited_regions[0] != this->regions[0] || visited_regions[1] != this->regions[1] ||
             this->extended_regions != nullptr)) {
            assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not." &&
                   (this->regions[1] != nullptr ? this->regions[0] != nullptr : 1));

            assert(
                "regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not." &&
                (this->extended_regions != nullptr ? this->regions[0] != nullptr && this->regions[1] != nullptr : 1));

            int v27 = 0;
            int v26 = 0;
            region *v13 = nullptr;
            for (auto *r = this->regions[0]; r != nullptr; r = v13) {
                if (!r->is_loaded()) {
                    ++v27;
                }

                if (++v26 >= 2) {
                    region *v4 = nullptr;
                    if (this->extended_regions != nullptr) {
                        region *v7 = nullptr;
                        if (v26 - 2 < static_cast<int>(this->extended_regions->size())) {
                            v7 = this->extended_regions->m_data[v26 - 2];
                        }

                        v4 = v7;
                    }

                    v13 = v4;
                } else {
                    v13 = this->regions[v26];
                }
            }

            auto exists_region = [](region **a1, int a2, region *a3) -> bool {
                for (int i = a2 - 1; i >= 0; --i) {
                    if (a1[i] == a3) {
                        return true;
                    }
                }

                return false;
            };

        LABEL_10:
            assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not." &&
                   (this->regions[1] != nullptr ? this->regions[0] != nullptr : 1));

            assert("regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not." &&
                   (this->extended_regions ? this->regions[0] != nullptr && this->regions[1] != nullptr : 1));

            assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

            int v7 = 0;

            region *v9 = nullptr;
            for (auto *r = this->regions[0]; r != nullptr; r = v9) {
                if (!exists_region(visited_regions, a3, r)) {
                    this->remove_me_from_region(r);
                    goto LABEL_10;
                }

                if (++v7 >= 2) {
                    v9 = (this->extended_regions != nullptr
                              ? ((v7 - 2) < static_cast<int>(this->extended_regions->size())
                                     ? this->extended_regions->m_data[v7 - 2]
                                     : nullptr)
                              : nullptr);
                } else {
                    v9 = this->regions[v7];
                }
            }

            for (int j = 0; j < a3; ++j) {
                auto *r = visited_regions[j];
                if (!this->is_in_region(r)) {
                    this->add_me_to_region(r);
                }
            }

            if (a3 > 0 && visited_regions[0]->is_loaded() && this->regions[0] != visited_regions[0]) {
                int k;
                for (k = 0; k < 2 && this->regions[k] != *visited_regions; ++k) {
                    ;
                }

                if (k >= 2) {
                    if (this->extended_regions != nullptr) {
                        for (auto m = 0u; m < this->extended_regions->size(); ++m) {
                            if (this->extended_regions->at(m) == visited_regions[0]) {
                                std::swap<region *>(this->regions[0], this->extended_regions->m_data[m]);
                                break;
                            }
                        }
                    }
                } else {
                    std::swap<region *>(this->regions[0], this->regions[k]);
                }
            }

            int loaded_count = 0;

            assert(this->regions[0] == visited_regions[0]);

            for (int i = 0; i < a3; ++i) {
                if (visited_regions[i]->is_loaded()) {
                    ++loaded_count;
                    assert(this->is_in_region(visited_regions[i]));
                }
            }

            assert(v27 || this->count_in_regions() == loaded_count);
        }
    } else {
        void(__fastcall * func)(void *, void *, region **, int) = CAST(func, 0x004F5510);
        return func(this, nullptr, visited_regions, a3);
    }
}

int entity::count_in_regions() const
{
    assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not." &&
           (this->regions[1] != nullptr ? this->regions[0] != nullptr : 1));

    assert("regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not." &&
           (this->extended_regions != nullptr ? this->regions[0] != nullptr && this->regions[1] != nullptr : 1));

    assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

    int v10 = 0;
    int v11 = 0;
    region *v8 = nullptr;
    for (auto *r = this->regions[0]; r != nullptr; r = v8) {
        ++v11;
        if (++v10 >= 2) {
            v8 = (this->extended_regions != nullptr ? (v10 - 2 < static_cast<int>(this->extended_regions->size())
                                                           ? this->extended_regions->m_data[v10 - 2]
                                                           : nullptr)
                                                    : nullptr);
        } else {
            v8 = this->regions[v10];
        }
    }

    return v11;
}

void entity::remove_from_regions()
{
    TRACE("entity::remove_from_regions");

    if constexpr (STANDALONE_SYSTEM) {
        assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not. " &&
               (this->regions[1] ? this->regions[0] != nullptr : 1));

        assert("regions_[ 0 ] and regions_[ 1 ] should not be NULL while extended_regions_ is not." &&
                       this->extended_regions
                   ? this->regions[0] && this->regions[1]
                   : 1);

        assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

        if (g_world_ptr != nullptr) {
            auto *v2 = this->regions[0];
            if (v2 != nullptr && v2->is_loaded()) {
                v2->remove(this);
            }

            auto *v3 = this->regions[1];
            if (v3 != nullptr && v3->is_loaded()) {
                v3->remove(this);
            }

            this->regions[1] = nullptr;
            this->regions[0] = nullptr;
            if (this->extended_regions != nullptr) {
                for (unsigned int v5 = 0; v5 < this->extended_regions->size(); ++v5) {
                    auto *reg = this->extended_regions->m_data[v5];
                    if (reg != nullptr && reg->is_loaded()) {
                        reg->remove(this);
                    }
                }

                entity_extended_regions_array_t::pool.remove(this->extended_regions);
                this->extended_regions = nullptr;
            }
        }
    } else {
        THISCALL(0x004CB750, this);
    }
}

region *entity::get_primary_region() const
{
    TRACE("entity::get_primary_region");

    if (!this->is_conglom_member()) {
        return this->regions[0];
    }

    entity *v2 = CAST(v2, this->get_conglom_owner());
    if (v2 == nullptr) {
        return nullptr;
    }

    return v2->get_primary_region();
}

bool entity::match_search_flags(int a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        const auto query = [this](unsigned offset) {
            return reinterpret_cast<bool (__fastcall *)(entity_base *, void *)>(
                get_vfunc(m_vtbl, offset))(this, nullptr);
        };
        if ((a2 & 1) == 0 && (((a2 & 0x20) == 0) || !(field_4 & 0x1000)) &&
            (((a2 & 0x40) == 0) || !query(0xCC)) && (((a2 & 0x80u) == 0) || !query(0xF0)) &&
            (((a2 & 0x200) == 0) || !query(0xA0)) &&
            (!query(0x64) ||
             ((((a2 & 4) == 0) || !query(0x114)) && (((a2 & 8) == 0) || !query(0x124)) &&
              (((a2 & 2) == 0) || get_ai_core() == nullptr))) &&
            (!(field_4 & 4) ||
             ((((a2 & 0x10) == 0) || !query(0x13C)) && (((a2 & 0x100) == 0) || !query(0x29C))))) {
            return false;
        }
        return ((((a2 & 0x800) == 0) || (field_4 & 0x200)) &&
                (((a2 & 0x1000) == 0) || !(field_4 & 0x200)) &&
                (((a2 & 0x2000) == 0) || query(0x50)) &&
                (((a2 & 0x4000) == 0) || !query(0x50)));
    } else {
        bool(__fastcall * func)(void *, void *edx, int) = CAST(func, 0x004C0970);
        return func(this, nullptr, a2);
    }
}

int entity::find_entities(int a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (found_entities == nullptr) {
            found_entities = new _std::list<entity *>;
        }

        found_entities->clear();

        auto *entities = g_world_ptr->ent_mgr.get_entities();
        auto it = entities->begin();
        auto end = entities->end();

        for (; it != end; ++it) {
            auto *ent = (*it);
            if (!ent)
                continue;

            if (ent->match_search_flags(a1)) {
                found_entities->push_back(ent);
            }
        }
        return found_entities->size();
    } else {
        return CDECL_CALL(0x004D67D0, a1);
    }
}

int entity::find_entities(unsigned flags, entity *center, float radius)
{

    if (!found_entities)
        found_entities = new _std::list<entity *>;
    found_entities->clear();
    const auto position = center->get_abs_position();
    auto *origin = center->get_primary_region();
    if (!origin)
        origin = g_world_ptr->the_terrain->find_region(position, nullptr);
    if (!origin)
        return 0;
    region_array nearby{};
    build_region_list_radius(&nearby, origin, position, radius, true);
    ++visit_key2;
    struct search_filter : local_collision::entfilter_base {
        uint32_t flags;
        static bool __fastcall accept(const local_collision::entfilter_base *base, void *, actor *value,
            dynamic_conglomerate_clone *, const local_collision::query_args_t *)
        {
            const auto flags = static_cast<const search_filter *>(base)->flags;
            if (flags & 0x4000)
                return value->colgeom->get_type() != 1;
            if (flags & 4)
                return (value->field_4 & 0x80000) != 0;
            if (flags & 2)
                return value->has_entity_collision();
            if (flags & 0x20) {
                bool character = value->get_ai_core() && !(value->field_4 & 0x800);
                if (!character && (value->field_4 & (0x8000 | 4))) {
                    auto *root = value->get_conglom_owner();
                    character = root->get_ai_core() && !(root->field_4 & 0x800);
                }
                return !(value->field_8 & 0x100000) && !character;
            }
            return true;
        }
    };
    search_filter filter;
    static local_collision::entfilter_base::native_vtable filter_table{&search_filter::accept};
    filter.m_vtbl = reinterpret_cast<std::intptr_t>(&filter_table);
    filter.flags = ((flags & 0x80000) |
        (((flags & 0x800000) | ((flags >> 2) & 0x180000)) >> 13)) >> 5;
    const auto add_candidate = [&](entity *candidate) {
        if (!candidate->match_search_flags(flags) ||
            !((candidate->get_abs_position() - position).length2() <= radius * radius))
            return;
        if (flags & 0xF00000) {
            vector3d hit, normal;
            entity *occluder = nullptr;
            auto *obb_filter = flags & 0x100000 ? local_collision::obbfilter_lineseg_test
                                              : local_collision::obbfilter_reject_all;
            if (find_intersection(position, candidate->get_abs_position(), filter, *obb_filter,
                &hit, &normal, nullptr, &occluder, nullptr, false) && occluder != candidate)
                return;
        }
        found_entities->push_back(candidate);
    };
    for (int index = 0; index < nearby.count; ++index) {
        auto &entities = *static_cast<_std::list<entity *> *>(nearby.m_data[index]->region_entities);
        for (auto it = entities.rbegin(); it != entities.rend(); ++it) {
            auto *candidate = *it;
            if (!candidate || candidate->field_64 == visit_key2)
                continue;
            candidate->field_64 = visit_key2;
            add_candidate(candidate);
            if (candidate->field_4 & 4)
                for (auto *member : static_cast<conglomerate *>(candidate)->members)
                    if (member->is_an_entity())
                        add_candidate(static_cast<entity *>(member));
        }
    }
    return found_entities->size();
}

void entity_patch()
{
    {
        FUNC_ADDRESS(address, &entity::update_regions);
        //SET_JUMP(0x004F5510, address);
    }

    {
        FUNC_ADDRESS(address, &entity::force_region_hack);
        SET_JUMP(0x0048B830, address);
    }

    {
        FUNC_ADDRESS(address, &entity::get_primary_region);
        SET_JUMP(0x004C0760, address);
    }

    {
        FUNC_ADDRESS(address, &entity::force_regions);
        //set_vfunc(0x0088F440, address);
    }
}
