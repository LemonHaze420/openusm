#pragma once

#include "progress.h"

#include <cstdint>

struct limited_timer;
struct worldly_pack_slot;

struct scene_entity_brew {
    progress field_0;
    limited_timer *field_4;
    worldly_pack_slot *field_8;
    int field_C;
    uint8_t *field_10;
    int buffer_index;
    int parse_code;
    int field_1C;
    int field_20;
    int field_24;
    progress field_28;
    int field_2C;
    int field_30;
    int field_34;
    progress field_38;
    int field_3C;
};
