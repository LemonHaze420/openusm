#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "signaller.h"

struct trigger_struct {
    vhandle_type<entity> handle;
    vector3d position;
    bool field_10;
    bool field_11;
    char padding[2];
};

struct trigger : signaller {
    float field_48;
    vhandle_type<entity> field_4C;
    _std::list<vhandle_type<entity>> *trigger_current_entities;
    trigger *m_next_trigger;

    //0x0050CBD0
    trigger(string_hash a2);

    //0x0056FE50
    ~trigger();

    //0x0053C470
    void update(trigger_struct *a2, int a3);

    void set_use_any_char(bool a2);

    void set_sees_dead_people(bool a2);

    entity *get_triggered_ent();

    //0x0053C390
    void set_multiple_entrance(bool a2);

    //virtual
    bool is_point_trigger() const;

    //virtual
    bool is_box_trigger() const;

    //virtual
    bool is_entity_trigger() const;
};
