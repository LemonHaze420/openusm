#pragma once

#include "vehicle_sounds.h"

#include "color32.h"
#include "entity.h"
#include "entity_base_vhandle.h"
#include "mstring.h"
#include "oldmath_po.h"
#include "variable.h"

inline constexpr auto VEHICLE_MODEL_TAXI = 0;
inline constexpr auto VEHICLE_MODEL_CAR = 1;
//2 => ?
//3 => ?
//4 => ?
//5 => ?
//6 => ?
//7 => ?
inline constexpr auto VEHICLE_MODEL_MAX = 8;

struct vehicle_model {
    int field_0;
    mString field_4;
    int refcount;
    float field_18;

    vehicle_model(int a1, mString a2);

    void sub_6B9F30(vhandle_type<entity> a2);

    //0x006C6680
    entity_base_vhandle create(int a3);
};

struct actor;

struct vehicle : vehicle_sounds {
    vhandle_type<entity> field_50;
    actor *field_54;
    uint32_t bodytype;
    int field_5C;
    int field_60;
    color32 field_64;
    int field_68;
    int field_6C;
    po field_70;
    vector3d field_B0;
    vector3d field_BC;
    float field_C8;
    float field_CC;
    float field_D0;
    int field_D4;
    int field_D8;
    int field_DC;
    float field_E0;
    int field_E4;
    float field_E8;
    float field_EC;
    float field_F0;
    float field_F4;
    int field_F8;
    actor *dftire;
    actor *pftire;
    actor *drtire;
    actor *prtire;
    actor *body;
    actor *field_110;
    actor *field_114;
    actor *field_118;
    actor *field_11C;
    actor *field_120;
    actor *field_124;
    actor *field_128;
    bool field_12C;
    bool field_12D;

    vehicle(vhandle_type<entity> a1);

    void reset();

    void update_part_cache();

    void sub_6D7EA0();

    vector3d sub_6DA250();

    void set_damage_level(int a2, int a3);

    int get_vehicle_body_type(vhandle_type<entity> a1);

    color32 get_part_color(string_hash a2);

    void set_collidable(bool a2);

    void set_visible(bool a2);

    void sub_6BAED0(const vector3d &pos);

    //virtual
    actor *get_my_actor();

    void determine_tire_radius();

    void determine_wheel_base();

    vector3d get_abs_position();

    void sub_6BA920(int a1);

    void pick_body_and_color();

    //0x006BA3A0
    void set_part_visible(string_hash a2, color32 a4, bool a5, bool a6, char a7);

    void set_nose_visible(int arg0, color32 a2, bool a4, int a5);

    //0x006BAF50
    void manage_vehicle_height(bool a1);

    static bool terminate_vehicles();

    static int pick_random_model();

    static int pick_model(int a1);

    //0x006C25D0
    static void pick_body_and_color(actor *a1);

    //0x006BA670
    void set_tail_visible(int a2, color32 a3, bool a4, int a5);

    //0x006BB350
    static void set_tail_visible(actor *a1, int a2, color32 a3, bool a4, int a5);

    //0x006BB260
    static void set_nose_visible(actor *a1, int a2, color32 a3, bool a4, int a5);

    //0x006B9F60
    static bool add_model(int a1, mString a2, Float a3);

    static Var<vehicle_model *[VEHICLE_MODEL_MAX]> models;

    static int &cur_vehicle_type;
};

extern void vehicle_patch();
