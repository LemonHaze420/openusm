#pragma once

#include "actor.h"

#include "float.hpp"
#include "combat_inode.h"
#include "sound_instance_id.h"

struct thrown_item;

struct grenade : actor {
    grenade *next;
    grenade *previous;
    bool inactive;
    bool armed;
    bool detonated;
    bool stuck;
    bool cleared_owner;
    bool first_frame;
    bool field_CE;
    bool redirected;
    vhandle_type<entity> weapon;
    float fuse;
    float arm_delay;
    float render_scale;
    float beam_timer;
    float stuck_time;
    sound_instance_id flight_sound;
    sound_instance_id armed_sound;
    ai::combat_inode::incoming_move incoming;
    vhandle_type<entity> visual;
    entity *beam_entity;
    float beam_length;
    vector3d previous_position;
    void *script_thread;
    int script_thread_id;

    static void *native_vtable(void **actor_table);
    void remove_from_list();
    void change_list_status();
    void clear(bool);
    void frame_advance(Float);
    void detonate(int, entity *);
    ~grenade();
    entity *check_hit(const vector3d &, const vector3d &, vector3d &, int *);
    bool check_if_hit(entity *, const vector3d &, const vector3d &, vector3d &, int *);
    void intercept();
    void redirect(const vector3d &, entity_base *);
    void redirect(entity_base_vhandle, entity_base *);
    void redirect_dir(const vector3d &, entity_base *);
    void reacquire_target(float, float, float, entity_base *);
    //0x00536580
    grenade(const string_hash &a2, unsigned int a3);

    void sub_4D6B10(int a2);

    //0x0051E070
    static void frame_advance_all_grenades(Float a3);
};
