#pragma once

#include "singleton.h"

#include "string_hash.h"
#include "vector3d.h"

struct box_trigger;
struct entity_base;
struct entity_trigger;
struct mString;
struct point_trigger;
struct trigger;
struct trigger_struct;

struct trigger_manager : singleton {
    trigger *m_triggers;

    trigger_manager();

    //0x0051E5A0
    void deinit();

    //0x0050CBA0
    void purge();

    //0x00541F30
    void update();

    //0x00541E90
    void update_trigger(trigger **a1, trigger_struct *a2, int a3);

    trigger *find_instance(entity_base *a2);

    trigger *find_instance(const mString &a2) const;

    void remove(trigger **trem);

    void delete_trigger(trigger *delete_me);

    void add_trigger(trigger *a2);

    point_trigger *new_point_trigger(vector3d a2, Float a5);

    point_trigger *new_point_trigger(string_hash a2, vector3d a3, Float a4);

    entity_trigger *new_entity_trigger(entity_base *a2, Float a3);

    box_trigger *new_box_trigger(string_hash a2, const vector3d &a3);

    box_trigger *new_box_trigger(string_hash a2, entity_base *a3);

    static void create_inst();

    static trigger_manager *&instance;
};

extern void trigger_manager_patch();
