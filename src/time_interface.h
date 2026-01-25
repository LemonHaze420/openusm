#pragma once

#include "entity_interface.h"

#include "float.hpp"

#include <cstdint>

struct entity;

struct time_interface : entity_interface {
    float field_C;
    float field_10;
    float field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;

    //0x004D9940
    time_interface(entity *a2);

    //0x004D18D0
    static void frame_advance_all_time_interfaces(Float a1);

    //0x004D9870
    void add_to_time_ifc_list();

    bool is_combat_dilated() const;

    double sub_4ADE50();
};
