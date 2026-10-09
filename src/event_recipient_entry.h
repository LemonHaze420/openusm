#pragma once

#include "entity_base_vhandle.h"

#include <vector.hpp>

struct event;
struct event_callback;
struct script_executable;
class script_instance;
struct vm_executable;

class event_recipient_entry {
    entity_base_vhandle field_0;
    _std::list<event_callback *> m_callbacks;
    _std::vector<void *> field_10;
    int field_20;
    int field_24;

public:
    event_recipient_entry(entity_base_vhandle a2, bool a3);

    ~event_recipient_entry();

    void *operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    auto get_my_vhandle() const
    {
        return this->field_0;
    }

    void clear();

    void clear_callbacks();

    //0x004C02A0
    int add_callback(script_instance *a2, const vm_executable *a3, char *a4, bool a5);

    //0x004D6260
    int add_callback(void (*cb)(event *, entity_base_vhandle, void *), void *a3, bool a4);

    bool callback_exists(int a2) const;

    void remove_callback(unsigned int a2);

    void clear_stale_callbacks();

    void clear_script_callbacks(script_executable *a2);

    void clear_script_callback(string_hash function);

    bool does_script_have_callbacks(const script_executable *executable) const;

    bool event_raised_last_frame(int ticks) const
    {
        return field_24 == ticks - 1 || (field_24 != ticks && field_20 == ticks - 1);
    }

    bool garbage_collect();

    void sub_4DB840(event *the_event);
};
