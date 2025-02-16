#pragma once

#include "entity_base_vhandle.h"
#include "string_hash.h"

#include <vector.hpp>

struct event;
struct event_recipient_entry;
struct event_type;
struct script_executable;

namespace event_manager {
    //0x004F3BE0
    extern void create_inst();

    //0x004EE7A0
    extern void clear();

    extern bool callback_exists(int id);

    //0x004F52B0
    extern void delete_inst();

    //0x004D1F40
    extern event_type *get_event_type(string_hash a1);

    //0x004C0440
    extern string_hash register_script_event_type(const char *a2, const char **a3);

    //0x004E19F0
    extern event_type *register_event_type(string_hash, bool);

    //0x004D2000
    extern bool does_script_have_callbacks(const script_executable *a1);

    //0x004EE9F0
    extern void raise_event(string_hash a1, entity_base_vhandle a2);

    //0x004EEA20
    extern void raise_event(event *event_to_raise, entity_base_vhandle a2);

    //0x004E1B00
    extern void garbage_collect();

    extern void clear_script_callbacks(entity_base_vhandle a1, script_executable *a2);

    extern event_recipient_entry * create_event_recipient(string_hash arg0, entity_base_vhandle a2);

    extern int add_callback(
        string_hash a1,
        entity_base_vhandle a2,
        void (*cb)(event *, entity_base_vhandle, void *),
        void *a4,
        bool a5);

    extern void remove_callback(unsigned int a1, string_hash a2, entity_base_vhandle a3);

    extern int & garbage_index;

    extern _std::vector<event_type *> & event_types;

}

extern void event_manager_patch();
