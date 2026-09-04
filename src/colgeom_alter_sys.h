#pragma once

#include "capsule.h"
#include "float.hpp"

struct actor;
struct entity_base;
struct conglomerate;
struct collision_capsule;

struct capsule_alter_sys {
    enum eAlterMode {};

    actor *field_0;
    eAlterMode field_4;
    collision_capsule *field_8;
    capsule base_rel_cap;
    capsule field_28;
    entity_base *field_44[4];
    int field_54;
    entity_base *field_58[2];
    int field_60;
    int field_64;
    int field_68;
    float field_6C[4];
    int field_7C;
    float field_80[2];
    int field_88;
    int field_8C;
    int field_90;

    float field_94;
    int field_98;
    int field_9C;
    bool field_A0;
    bool field_A1;
    bool field_A2;
    bool dynamic;

    //0x005C51D0
    capsule_alter_sys(actor *a2);

    //0x005BA2F0
    void compute_avg_values();

    //0x005D7170
    void adjust_colgeom(bool a2);

    //0x005D94F0
    void set_mode(eAlterMode a2);

    //0x005D9540
    void set_static_capsule(const vector3d &a2, const vector3d &a3, Float a4);

    //0x005BA2D0
    void set_avoid_floor(bool a2);

    //0x005BA2A0
    void set_avg_radius(Float a2);

    //0x005BA250
    void set_base_avg_node(int index, entity_base *a3, Float a4);

    //0x005BA270
    void set_end_avg_node(int index, entity_base *a3, Float a4);
};

//0x00687B70
extern void set_to_default_capsule_alter(capsule_alter_sys *a1, conglomerate *a2);
