#include "entity.h"

#include "collision_geometry.h"
#include "common.h"
#include "conglom.h"
#include "fixed_pool.h"
#include "fixed_vector.h"
#include "func_wrapper.h"
#include "memory.h"
#include "moved_entities.h"
#include "region.h"
#include "time_interface.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "wds.h"

#include <cassert>

VALIDATE_SIZE(entity, 0x68u);

namespace entity_extended_regions_array_t {
    static fixed_pool & pool = var<fixed_pool>(0x0091FF9C);
}

entity::entity(const string_hash &a2, uint32_t a3) : signaller(a2, a3, false) {
    this->field_64 = 0;
    this->field_60 = 0;
    this->field_5C = 0;
    this->regions[0] = nullptr;
    this->regions[1] = nullptr;
    this->extended_regions = nullptr;
    this->field_58 = nullptr;
    this->colgeom = nullptr;
}

void entity::destroy_static_entity_pointers() {
    CDECL_CALL(0x004D6940);
}

entity::~entity()
{
    if constexpr (0)
    {
        this->remove_from_regions();
        if ( this->has_time_ifc() )
        {
            auto *v2 = this->field_58;
            if ( v2->field_8 )
            {
                if ( v2 != nullptr ) {
                    void (__fastcall *finalize)(void *, void *, bool) = CAST(finalize, get_vfunc(v2->m_vtbl, 0x0));
                    finalize(v2, nullptr, true);
                }
            }
            else
            {
                void (__fastcall *func)(void *) = CAST(func, get_vfunc(v2->m_vtbl, 0x24));
                func(v2);
            }

            this->field_58 = nullptr;
        }
    }
    else
    {
        THISCALL(0x004F91C0, this);
    }
}

void entity::randomize_position(const vector3d &a2, Float a3, Float a4, Float a5)
{
    TRACE("entity::randomize_position");

    THISCALL(0x004E2460, this, &a2, a3, a4, a5);
}

void entity::update_proximity_maps()
{
    TRACE("entity::update_proximity_maps");
    
    THISCALL(0x004CB810, this);
}

bool entity::is_in_limbo() const
{
    auto v1 = [](const entity *self) -> bool
    {
        return self->is_ext_flagged(0x2000u) || !bit_cast<entity *>(self)->get_primary_region();
    }(this);

    auto sub_6A7DAB = [](const entity_base *self) -> bool
    {
        return self->is_flagged(8u);
    };
    return v1 && !sub_6A7DAB(this);
}

float entity::get_visual_radius()
{
    if constexpr (0)
    {
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
        float (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x28));
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
        assert(!is_a_conglomerate() && !is_an_actor() && !is_conglom_member());

        result = this->get_abs_position();
    }

    return result;
}

void entity::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("entity::un_mash");

    for (auto i = 0; i < 2; ++i)
    {
        this->regions[i] = nullptr;
    }

    this->extended_regions = nullptr;
    this->field_58 = nullptr;
    this->colgeom = nullptr;
    entity_base::un_mash(a2, a3, a4);
}

void entity::clear_region(region *r, int i_know_what_i_am_doing) {
    THISCALL(0x004F54A0, this, r, i_know_what_i_am_doing);
}

entity *entity::compute_sector(terrain *a1, bool a2, entity *a3)
{
    if constexpr (0)
    {
        entity *result;

        if ((this->field_4 & 0x10000000) == 0) {
            moved_entities::add_moved({this->get_my_handle()});
        }

        return result;
    } else {
        entity * (__fastcall *func)(void *, void *, terrain *, bool, entity *) = CAST(func, get_vfunc(m_vtbl, 0x16C));
        return func(this, nullptr, a1, a2, a3);
    }
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
        if ( this->is_flagged(0x10000000) ) {
            this->remove_from_regions();
        }

        this->set_flag_recursive(static_cast<entity_flag_t>(0x10000000u), true);
        if (r != nullptr) {
            if (!this->is_in_region(r)) {
                this->add_me_to_region(r);
            }
        }
    } else {
        void (__fastcall *func)(entity *, void *, region *) = CAST(func, get_vfunc(m_vtbl, 0x174));
        func(this, nullptr, r);
    }
}

void entity::force_current_region() {
    this->set_flag_recursive(static_cast<entity_flag_t>(0x10000000u), true);
}

void entity::unforce_regions()
{
    if ( this->is_flagged(0x10000000) ) {
        this->remove_from_regions();
        this->set_flag_recursive(static_cast<entity_flag_t>(0x10000000), false);
    }
}

void entity::force_regions(entity *ent) {
    THISCALL(0x004F56A0, this, ent);
}

void entity::update_ai_proximity_map_recursive() {
    ;
}

void entity::set_family_visible(bool a2) {
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

bool entity::is_renderable() const {
    return this->is_flagged(0x100u);
}

bool entity::possibly_collide()
{
    if constexpr (1)
    {
        bool (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x190));
        return func(this);
    }
    else
    {
        return this->get_colgeom() != nullptr && this->are_collisions_active();
    }
}

bool entity::possibly_camera_collide()
{
    auto *v1 = this->get_colgeom();
    return v1 != nullptr && this->are_collisions_active() && (v1->field_C & 0x10) != 0;
}

bool entity::possibly_walkable() const
{
    bool result = false;
    if (this->colgeom != nullptr)
    {
        if (this->are_collisions_active() && this->is_walkable()) {
            result = true;
        }
    }

    return result;
}

bool entity::sub_4C08E0() {
    return this->colgeom != nullptr && this->are_collisions_active() &&
        !this->is_ext_flagged(0x8000);
}

void entity::region_update_poss_collide() {
    THISCALL(0x004C0810, this);
}

void entity::frame_advance(Float) {
    ;
}

bool entity::is_still_visible() {
    return (this->field_4 & 0x200) != 0;
}

void entity::render(Float a2) {
    void (__fastcall *func)(entity *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x1AC));

    func(this, nullptr, a2);
}

nglMesh *entity::get_mesh()
{
    if constexpr (0)
    {
        return nullptr;
    }
    else
    {
        nglMesh * (__fastcall *func)(const entity *) = CAST(func, get_vfunc(m_vtbl, 0x1B0));

        return func(this);
    }
}

bool entity::has_mesh() {
    return false;
}

void entity::suspend(bool) {
    ;
}

void entity::unsuspend(bool) {
    ;
}

void entity::set_render_color(color32 c)
{
    void (__fastcall *func)(entity *, void *, color32) = CAST(func, get_vfunc(m_vtbl, 0x1C0));
    func(this, nullptr, c);
}


color32 entity::get_render_color() const
{
    if constexpr (1)
    {
        color32 (__fastcall *func)(const entity *) = CAST(func, get_vfunc(m_vtbl, 0x1C4));

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

void entity::set_render_alpha_mod(Float) {
    ;
}

float entity::get_render_alpha_mod() const
{
    if constexpr (1) {
        float (__fastcall *func)(const entity *) = CAST(func, get_vfunc(m_vtbl, 0x1CC));

        return func(this);

    } else {
        return 0.0;
    }
}

void entity::set_render_scale(const vector3d &) {
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

void entity::set_render_zbias(Float) {
    ;
}

float entity::get_render_zbias() const {
    return 0.0;
}

light_manager *entity::get_light_set() const {
    light_manager * (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x1E0));
    return func(this);
}

bool entity::can_be_a_lego() {
    return false;
}

bool entity::can_be_a_conglom_clone() {
    return false;
}

bool entity::is_a_dynamic_conglomerate_clone() {
    return false;
}

void entity::set_collisions_active(bool a1, bool a2) {
    void (__fastcall *func)(entity *, void *, bool, bool) = CAST(func, get_vfunc(m_vtbl, 0x1F0));

    func(this, nullptr, a1, a2);
}

bool entity::are_character_collisions_active() {
    return false;
}

void entity::set_character_collisions_active(bool a2) {
    if (a2) {
        this->field_4 |= 0x10;
    } else {
        this->field_4 &= 0xFFFFFFEF;
    }
}

void entity::set_terrain_collisions_active(bool a2) {
    if (a2) {
        this->field_4 |= 0x20;
    } else {
        this->field_4 &= 0xFFFFFFDF;
    }
}

float entity::get_age() {
    return 0.0;
}

void entity::set_age(Float) {
    ;
}

void entity::set_recursive_age(Float a2) {
    this->set_age(a2);
}

bool entity::is_in_region(const region *r) const
{
    if constexpr (1) {
        assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not. "
                    && ( this->regions[ 1 ] ? this->regions[ 0 ] != nullptr : 1 ));

        assert("regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not."
                    && this->extended_regions ? this->regions[ 0 ] && this->regions[ 1 ] : 1);

        assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

        auto v4 = 0;
        region *v2 = nullptr;
        for (auto *reg = this->regions[0]; reg != r; reg = v2)
        {
            if (reg == r) {
                return true;
            }

            if ( ++v4 >= 2 )
            {
                if ( this->extended_regions != nullptr ) {
                    v2 = ( (v4 - 2) < static_cast<int>(extended_regions->size())
                                    ? extended_regions->m_data[v4 - 2]
                                    : nullptr
                                    );
                }
            }
            else
            {
                v2 = this->regions[v4];
            }
        }

        return false;
    } else {
        return (bool) THISCALL(0x004CB5C0, this, r);
    }
}

void entity::add_me_to_region(region *r)
{
    TRACE("entity::add_me_to_region");

    if constexpr (1) {
        assert((!is_a_conglomerate() || !((conglomerate *)this)->is_cloned_conglomerate()) && "A cloned conglomerate should NEVER be put into a region!!!");

        assert(r != nullptr);

        auto ent_to_add = this->get_my_vhandle();
        assert(ent_to_add != INVALID_HANDLE);

        if ( !this->is_in_region(r) )
        {
            if ( r->is_loaded() )
            {
                int i = 0;
                for (; i < 2; ++i)
                {
                    if ( this->regions[i] == nullptr ) {
                        this->regions[i] = r;
                        break;
                    }
                }

                if (i == 2) {
                    if ( this->extended_regions == nullptr ) {
                        auto *mem = entity_extended_regions_array_t::pool.allocate_new_block();
                        auto *v5 = new (mem) fixed_vector<region *, 7> {};
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

collision_geometry *entity::get_colgeom() const {
    return this->colgeom;
}

float entity::get_colgeom_radius() const {
    float (*func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x254));

    return func(this);
}

vector3d entity::get_colgeom_center() const
{
    void (__fastcall *func)(const void *, void *, vector3d *) = CAST(func, get_vfunc(m_vtbl, 0x258));

    vector3d result;
    func(this, nullptr, &result);

    return result;
}

void entity::remove_me_from_region(region *r) {
    assert(r != nullptr);

    THISCALL(0x004F5390, this, r);
}

void entity::create_time_ifc() {
    if constexpr (1) {
        auto *mem = mem_alloc(sizeof(time_interface));

        this->field_58 = new (mem) time_interface{this};
    } else {
        THISCALL(0x004DBA10, this);
    }
}

bool entity::is_indoors() {
    return (bool) THISCALL(0x004CB670, this);
}

void entity::update_regions(region **visited_regions, int a3)
{
    if constexpr (1)
    {
        if ( (a3 != 1 || visited_regions[0] != this->regions[0] || this->regions[1] != nullptr)
                && ( a3 != 2 || visited_regions[0] != this->regions[0] || visited_regions[1] != this->regions[1] || this->extended_regions != nullptr ) )
        {
            assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not."
                    && ( this->regions[ 1 ] != nullptr ? this->regions[ 0 ] != nullptr : 1 ));

            assert("regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not." 
                    && (this->extended_regions != nullptr ? this->regions[ 0 ] != nullptr && this->regions[ 1 ] != nullptr : 1) );

            int v27 = 0;
            int v26 = 0;
            region *v13 = nullptr;
            for ( auto *r = this->regions[0]; r != nullptr; r = v13 )
            {
                if ( !r->is_loaded() ) {
                    ++v27;
                }

                if ( ++v26 >= 2 )
                {
                    region *v4 = nullptr;
                    if ( this->extended_regions != nullptr )
                    {
                        region *v7 = nullptr;
                        if ( v26 - 2 < static_cast<int>(this->extended_regions->size()) ) {
                            v7 = this->extended_regions->m_data[v26 - 2];
                        }

                        v4 = v7;
                    }

                    v13 = v4;
                }
                else
                {
                    v13 = this->regions[v26];
                }
            }

            auto exists_region = [](region **a1, int a2, region *a3) -> bool
            {
                for ( int i = a2 - 1; i >= 0; --i )
                {
                    if ( a1[i] == a3 ) {
                        return true;
                    }
                }

                return false;
            };

LABEL_10:
            assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not."
                    && ( this->regions[ 1 ] != nullptr ? this->regions[ 0 ] != nullptr : 1 ));

            assert("regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not."
                    && (this->extended_regions ? this->regions[ 0 ] != nullptr && this->regions[ 1 ] != nullptr : 1) );

            assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

            int v7 = 0;

            region *v9 = nullptr;
            for ( auto *r = this->regions[0]; r != nullptr; r = v9)
            {
                if ( !exists_region(visited_regions, a3, r) )
                {
                    this->remove_me_from_region(v9);
                    goto LABEL_10;
                }

                if (++v7 >= 2) {
                    v9 = ( this->extended_regions != nullptr
                            ? ( (v7 - 2) < static_cast<int>(this->extended_regions->size()) ? this->extended_regions->m_data[v7 - 2] : nullptr )
                            : nullptr );
                } else {
                    v9 = this->regions[v7];
                }
            }

            for ( int j = 0; j < a3; ++j )
            {
                auto *r = visited_regions[j];
                if ( !this->is_in_region(r) ) {
                    this->add_me_to_region(r);
                }
            }

            if ( a3 > 0 && visited_regions[0]->is_loaded() && this->regions[0] != visited_regions[0] )
            {
                int k;
                for ( k = 0; k < 2 && this->regions[k] != *visited_regions; ++k ) {
                    ;
                }

                if (k >= 2)
                {
                    if ( this->extended_regions != nullptr )
                    {
                        for ( auto m = 0u; m < this->extended_regions->size(); ++m )
                        {
                            if ( this->extended_regions->at(m) == visited_regions[0] ) {
                                std::swap<region *>(this->regions[0], this->extended_regions->m_data[m]);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    std::swap<region *>(this->regions[0], this->regions[k]);
                }
            }

            int loaded_count = 0;

            assert(this->regions[ 0 ] == visited_regions[ 0 ]);

            for ( int i = 0; i < a3; ++i )
            {
                if ( visited_regions[i]->is_loaded() )
                {
                    ++loaded_count;
                    assert(this->is_in_region( visited_regions[ i ] ));
                }
            }

            assert(v27 || this->count_in_regions() == loaded_count);
        }
    }
    else
    {
        void (__fastcall *func)(void *, void *, region **, int) = CAST(func, 0x004F5510);
        return func(this, nullptr, visited_regions, a3);
    }
}

int entity::count_in_regions() const
{
    assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not."
            && ( this->regions[ 1 ] != nullptr ? this->regions[ 0 ] != nullptr : 1 ) );

    assert("regions[ 0 ] and regions[ 1 ] should not be NULL while extended_regions is not."
            && (this->extended_regions != nullptr ? this->regions[ 0 ] != nullptr && this->regions[ 1 ] != nullptr : 1) );

    assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

    int v10 = 0;
    int v11 = 0;
    region *v8 = nullptr;
    for ( auto *r = this->regions[0]; r != nullptr; r = v8 )
    {
        ++v11;
        if ( ++v10 >= 2 )
        {
            v8 = ( this->extended_regions != nullptr
                    ? ( v10 - 2 < static_cast<int>(this->extended_regions->size()) ? this->extended_regions->m_data[v10 - 2] : nullptr )
                    : nullptr
                 );
        }
        else
        {
            v8 = this->regions[v10];
        }
    }

    return v11;
}

void entity::remove_from_regions()
{
    TRACE("entity::remove_from_regions");

    if constexpr (0)
    {
        assert("regions[ 0 ] can not be NULL when regions[ 1 ] is not. "
                    && ( this->regions[ 1 ] ? this->regions[ 0 ] != nullptr: 1 ));

        assert("regions_[ 0 ] and regions_[ 1 ] should not be NULL while extended_regions_ is not."
                    && this->extended_regions ? this->regions[ 0 ] && this->regions[ 1 ] : 1);

        assert(this->extended_regions != nullptr ? this->extended_regions->size() > 0 : 1);

        if ( g_world_ptr != nullptr )
        {
            auto *v2 = this->regions[0];
            if ( v2 != nullptr && v2->is_loaded() ) {
                v2->remove(this);
            }

            auto *v3 = this->regions[1];
            if ( v3 != nullptr && v3->is_loaded() ) {
                v3->remove(this);
            }

            this->regions[1] = nullptr;
            this->regions[0] = nullptr;
            if ( this->extended_regions != nullptr )
            {
                for (auto v5 = 0; v5 < this->extended_regions->size(); ++v5)
                {
                    auto *reg = this->extended_regions->m_data[v5];
                    if ( reg != nullptr && reg->is_loaded() ) {
                        reg->remove(this);
                    }
                }

                entity_extended_regions_array_t::pool.remove(this->extended_regions);
                this->extended_regions = nullptr;
            }
        }
    }
    else
    {
        THISCALL(0x004CB750, this);
    }
}

region *entity::get_primary_region() const
{
    TRACE("entity::get_primary_region");

    if ( !this->is_conglom_member() ) {
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
    if constexpr (0)
    {
        if ( (a2 & 1) == 0
            && (((a2 & 0x20) == 0) || !this->is_flagged(0x1000))
            && (((a2 & 0x40) == 0) || !this->is_a_switch_obj())
            && (((a2 & 0x80u) == 0) || !this->is_a_grenade())
            && (((a2 & 0x200) == 0) || !this->is_a_water_exit_marker())
            && (!this->is_an_actor()
            || ( (((a2 & 4) == 0) || !this->has_damage_ifc())
            && (((a2 & 8) == 0) || !this->has_physical_ifc())
            && (((a2 & 2) == 0) || this->get_ai_core() == nullptr)) )
            && (!this->is_a_conglomerate()
            || ((((a2 & 0x10) == 0) || !this->has_script_data_ifc())
            && (((a2 & 0x100) == 0) || !bit_cast<conglomerate *>(this)->has_variant_ifc() )) ) )
        {
            return false;
        }

        return ( (((a2 & 0x800) == 0) || this->is_visible())
            && (((a2 & 0x1000) == 0) || !this->is_visible())
            && (((a2 & 0x2000) == 0) || this->is_alive())
            && (((a2 & 0x4000) == 0) || !this->is_alive()) );
    }
    else
    {
        bool (__fastcall *func)(void *, void *edx, int) = CAST(func, 0x004C0970);
        return func(this, nullptr, a2);
    }
}

int entity::find_entities(int a1)
{
    if constexpr (0)
    {
        if ( found_entities == nullptr ) {
            found_entities = new _std::list<entity *>;
        }

        found_entities->clear();

        auto *entities = g_world_ptr->ent_mgr.get_entities();
        auto it = entities->begin();
        auto end = entities->end();

        for ( ; it != end; ++it )
        {
            auto *ent = (*it);
            assert(ent != nullptr);

            if ( ent->match_search_flags(a1) ) {
                found_entities->push_back(ent);
            }
        }
        return found_entities->size();
    }
    else
    {
        return CDECL_CALL(0x004D67D0, a1);
    }
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
