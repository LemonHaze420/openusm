#pragma once

#include "item.h"

#include "visual_item.h"

struct entity_base;

struct handheld_item : item {
    int field_100;
    vhandle_type<visual_item> field_104;
    entity_base *field_108;
    int field_10C;
    bool field_110;
};
