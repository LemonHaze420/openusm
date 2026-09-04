#pragma once

#include "entity_base_vhandle.h"

#include <vector.hpp>

struct event;
struct event_callback;
struct script_executable;
struct script_instance;
struct vm_executable;

struct event_recipient_entry {
    entity_base_vhandle field_0;
    _std::list<event_callback *> field_4;
    _std::vector<void *> field_10;
    int field_20;
    int field_24;

    event_recipient_entry(entity_base_vhandle a2, bool a3);

    ~event_recipient_entry();

    void *operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    void clear();

    void clear_callbacks();

    //0x004C02A0
    int add_callback(script_instance *a2, const vm_executable *a3, char *a4, bool a5);

    //0x004D6260
    int add_callback(void (*cb)(event *, entity_base_vhandle, void *), void *a3, bool a4);

    bool callback_exists(int a2) const;

    void remove_callback(unsigned int a2);

    void clear_script_callbacks(script_executable *a2);
};
