#pragma once

#include "handheld_item.h"
#include "cached_special_effect.h"
#include "color32.h"
#include "beam.h"
#include <list.hpp>

struct beam;
struct gun_beam_cache;
struct blaster_beam;
struct nglTexture;
struct gun;

struct gun_hit {
    entity *target;
    float damage;
    vector3d position;
    vector3d direction;
    int count;
    float distance_squared;
};

struct gun_beam {
    vhandle_type<beam> handle;
    vector3d offset;
    color32 color;
    float width;
    nglTexture *texture;
    float lifetime;
    float speed;
    float length;
    float texture_scale;
    bool additive;
    bool cached;
    gun_beam_cache *cache;

    void un_mash(generic_mash_header *, gun *, generic_mash_data_ptrs *);
    blaster_beam *spawn(bool, const vector3d &, const vector3d &, gun *,
                        _std::list<blaster_beam *> *, void *);
};

struct gun : handheld_item {
    _std::list<gun_hit> *hits;
    float damage;
    int magazine_size;
    int ammunition;
    int shots_fired;
    bool charge_effect_active;
    bool charge_effect_dirty;
    bool field_12A;
    bool field_12B;
    int field_12C;
    float impulse;
    float shot_interval;
    float shot_cooldown;
    float range;
    int field_140;
    float fire_delay;
    float fire_countdown;
    entity *pending_source;
    void *pending_move;
    float spread;
    int pellets;
    int explosive_hit_count;
    vhandle_type<entity> tracked_target;
    entity *target;
    vector3d target_position;
    vector3d target_direction;
    vector3d effect_offset;
    mString fire_script;
    mString flight_script;
    cached_special_effect charge_effect;
    cached_special_effect muzzle_effect;
    cached_special_effect hit_effect;
    cached_special_effect world_effect;
    cached_special_effect blocked_effect;
    float damage_radius;
    float direct_damage_multiplier;
    float trail_length;
    gun_beam projectile;
    gun_beam laser;
    _std::list<blaster_beam *> *blasters;
    bool reloaded;
    bool field_365;
    bool hit_target;
    unsigned char field_367;
    int penetrations;
    int max_targets;
    float aim_cone;

    static void *native_vtable(void **handheld_table);
    ~gun();
    void release_mem();
    void un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *);
    void frame_advance(Float);
    void apply_effects(actor *);
    void holster(bool);
    void draw(bool);
    void hide();
    void show();
    void set_visibility(bool);
    void fire_at_position(vhandle_type<entity>, const vector3d &, void *);
    void fire_at_target(vhandle_type<entity>, vhandle_type<entity>, void *);
    void do_effects(entity *, entity *, const vector3d &, const vector3d &, string_hash);
    void update_continuous_fire();
    void deactivate_continuous_fire();
    vector3d calculate_fire_pos();
    vector3d calculate_laser_pos();
    vector3d fire_direction();
    void calculate_target(float, bool, const vector3d &, bool);
    void update_targeting();
    bool can_hit_this_target(entity *);
    void setup_fire_gun(entity *, void *);
    void check_fire_gun();
    void fire_gun(entity *, void *);
    void accumulate_hit(entity *, const vector3d &, const vector3d &);
    void update_charge_effect();
    void activate_laser();
    void remove_laser();
    void update_blasters(float);
    void release_blasters(bool);
};
