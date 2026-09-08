#pragma once

#include "entity_interface.h"

#include "float.hpp"

#include <cstdint>

struct entity;

struct time_interface : entity_interface {
    float field_C;
    float field_10;
    float field_14;
    float field_18;
    float field_1C;
    float field_20;
    float field_24;
    float field_28;
    int field_2C;
    int field_30;

    //0x004D9940
    time_interface(entity *a2);
    ~time_interface();

    //0x004D18D0
    static void frame_advance_all_time_interfaces(Float a1);

    //0x004D9870
    void add_to_time_ifc_list();
    void frame_advance(Float a1);

    void set_state(int state, Float elapsed);

    bool is_combat_dilated() const;

    double sub_4ADE50();
};
