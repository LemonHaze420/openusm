#pragma once

#include "handheld_item.h"
#include "cached_special_effect.h"
#include "combat_inode.h"

struct grenade;
struct thrown_item;

struct grenade_cache {
    _std::vector<vhandle_type<entity>> *entries;
    int references;
    int field_8;
    int initial_count;

    void release_mem();
    void un_mash(thrown_item *, generic_mash_data_ptrs *);
    grenade *push_new_grenade(thrown_item *);
    grenade *get_grenade(thrown_item *);
};

struct thrown_item : handheld_item {
    int field_114;
    float field_118;
    float field_11C;
    int field_120;
    int field_124;
    float field_128;
    float fuse;
    float fuse_variation;
    float arm_delay;
    float field_138;
    float field_13C;
    cached_special_effect field_140;
    cached_special_effect field_180;
    cached_special_effect field_1C0;
    cached_special_effect field_200;
    cached_special_effect field_240;
    vector3d launch_direction;
    float launch_speed;
    float gravity_multiplier;
    vector3d detonate_position;
    vector3d effect_offset;
    grenade *last_grenade;
    grenade *field_2B0;
    grenade *field_2B4;
    resource_key projectile_resource;
    vhandle_type<entity> projectile_template;
    float guidance_probability;
    float field_2C8;
    float prediction;
    float target_spread;
    float field_2D4;
    float field_2D8;
    float field_2DC;
    int field_2E0;
    int field_2E4;
    int field_2E8;
    int field_2EC;
    int field_2F0;
    vector3d spin;
    float field_300;
    float field_304;
    float field_308;
    float field_30C;
    float field_310;
    float field_314;
    grenade_cache *cache;
    void *field_31C;
    thrown_item *mirv;
    int mirv_count;
    float mirv_lateral;
    float mirv_vertical;
    float field_330;
    float spread;
    bool field_338;
    _std::vector<grenade *> *live_grenades;
    mString field_340;

    static inline Var<_std::vector<grenade *>> all_grenades{0x0095FF84};
    static inline Var<bool> target_valid{0x0095C764};
    static inline Var<vhandle_type<entity>> target_ent{0x0095FBA0};
    static inline Var<vector3d> target_hit{0x0095CB2C};
    static inline Var<vector3d> target_norm{0x00960170};

    static void *native_vtable(void **handheld_table);
    ~thrown_item();
    bool can_damage(entity_base *, bool redirected) const;
    bool is_trip_mine() const;
    void release_mem();
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);
    void frame_advance(Float);
    void set_owner(actor *);
    void internal_apply_effects(actor *, ai::combat_inode::incoming_move *);
    vector3d invert_launch_vec(const vector3d &);
    vector3d *fire_at_target_internal(vhandle_type<entity>, vhandle_type<entity>, const vector3d &,
                                      ai::combat_inode::incoming_move *);
    void spawn_grenade(vector3d, float, bool, const vector3d &, ai::combat_inode::incoming_move *);
    grenade *get_new_grenade();
    void spawn_mirvs(int, const vector3d &, const vector3d &, vhandle_type<entity>, const vector3d &, const vector3d &,
                     ai::combat_inode::incoming_move *);
    static vector3d calc_target_pos_delta(float);
    static vector3d calc_target_pos(const vector3d &, vhandle_type<entity>, float, const vector3d &, float);
    static void remove_live_grenade(grenade *);
    static void clear_all();
};
