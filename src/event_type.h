#pragma once

#include "entity_base_vhandle.h"
#include "string_hash.h"

struct event;
struct event_callback;
struct event_recipient_entry;
struct script_executable;

struct event_type {
    string_hash field_0;
    event *event_to_raise;
    _std::vector<event_recipient_entry *> field_8;
    bool field_18;
    _std::list<event_callback *> field_1C;
    bool field_28;

    //0x004E18B0
    event_type(string_hash a2, bool a3);

    //0x004E1970
    ~event_type();

    void * operator new(size_t sz);

    void operator delete(void *ptr, size_t sz);

    //0x004DB890
    void clear();

    //0x004D1ED0
    void clear_callbacks();

    event_recipient_entry * find_recipient_entry(entity_base_vhandle a2);

    //0x004EE6C0
    void raise_event(entity_base_vhandle a2, event *a3);

    //0x004EE620
    event_recipient_entry *create_recipient_entry(entity_base_vhandle a2);

    //0x004CFB90
    bool callback_exists(int a2) const;

    void remove_default_callback(unsigned int a2);

    void clear_script_callbacks(entity_base_vhandle a2, script_executable *a3);

    bool garbage_collect();
};
