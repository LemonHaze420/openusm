#pragma once

#include "entity_base_vhandle.h"
#include "mstring.h"

#include <list>

struct debug_menu;

namespace script_memtrack {

struct entity_class_t {
    mString field_0;
    int field_C;
    std::list<std::pair<entity_base_vhandle, int>> field_10;
};

void begin_entity_creation(const mString &a1);

void frame_advance();

void create_debug_menu(debug_menu *arg0);
}  // namespace script_memtrack

extern void script_memtrack_patch();
