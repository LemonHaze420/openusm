#pragma once

#include "generic_interface.h"
#include "entity_base_vhandle.h"

#include "cached_special_effect.h"
#include "float.hpp"
#include "vector3d.h"

#include <list.hpp>
#include <vector.hpp>

struct actor;
struct resource_key;
struct entity;

template <typename T>
struct bounded_variable {
    T field_0[4];

    void sub_48BFB0(const T &a2);
};

struct damage_info {
    float amount;
    vector3d field_4;
    vector3d field_8;
    vector3d field_C;
    int field_28;
    int field_2C;
    entity_base_vhandle field_30;
    entity_base_vhandle field_34;
    int field_38;
    bool field_3C;
    bool field_3D;
    bool field_3E;
    bool field_3F;
};

struct damage_interface : generic_interface {
    actor *field_4;
    bool dynamic;
    mString field_C;
    mString field_1C;
    mString field_2C;
    mString field_3C;
    struct morph_region {
        resource_key member;
        int threshold;
        int accumulated_damage;
    } morph_regions[6];
    char morph_source[32];
    resource_key morph_mesh;
    int morph_registration_id;
    string_hash field_D8;
    vector3d prop_velocity;
    float prop_velocity_randomness;
    float prop_lifetime;
    float explosion_inner_radius;

    bool field_F4;
    float field_F8;
    resource_key field_FC;
    damage_info field_104;
    damage_info field_144;
    cached_special_effect field_184;
    float field_1C4;
    string_hash field_1C8;
    void *field_1CC;
    void *field_1D0;
    float field_1D4;
    float field_1D8;
    int field_1DC;
    int field_1E0;
    float field_1E4;
    float field_1E8;
    float field_1EC;
    float field_1F0;
    bool field_1F4;
    bool field_1F5;
    int field_1F8;
    bounded_variable<float> field_1FC;
    bounded_variable<float> field_20C;
    bounded_variable<float> field_21C;
    bounded_variable<int> field_22C;

    bool is_alive() const
    {
        return field_1FC.field_0[0] > 0.0f;
    }
    bool is_subdued() const
    {
        return field_21C.field_0[0] > 0.0001f && field_1FC.field_0[0] < 0.0001f;
    }

    //0x004DE8A0
    damage_interface(actor *a2);

    //0x004D9BF0
    ~damage_interface();

    void remove_from_dmg_ifc_list();

    /* virtual */ bool get_ifc_num(const resource_key &att, float *a3, bool is_log);

    /* virtual */ bool set_ifc_num(const resource_key &att, Float a3, bool is_log);

    //virtual
    void _un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5);

    //virtual
    void release_ifc();

    //virtual
    void frame_advance(Float a3);


    void apply_damage(entity *source, float amount, int damage_type, const vector3d &position,
                      const vector3d &direction, int flags, const string_hash &attack, const string_hash &category,
                      const string_hash &reaction, bool force_reaction, const vector3d &target, int combo_type,
                      bool skip_combat);
    void post_destruction_actions();
    void continue_post_destruction_actions();
    void apply_subdue(entity *source, float amount);


    void update_hp_change(Float time_step);

    //0x004D1990
    static void frame_advance_all_damage_ifc(Float a1);


    static int find_damageable(const vector3d &position, float radius, unsigned flags, bool restrict_regions);

    static inline auto &all_damage_interfaces = var<_std::vector<damage_interface *> *>(0x0095A660);

    static inline auto &found_damageable = var<_std::list<damage_interface *> *>(0x0095A5EC);
};


extern void damage_interface_patch();
