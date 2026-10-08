#pragma once

#include "event_callback.h"

#include "entity_base_vhandle.h"

struct event;

struct code_event_callback : event_callback {
    void (*m_callback)(event *, entity_base_vhandle, void *);

    code_event_callback(void (*a2)(event *, entity_base_vhandle, void *), void *a1, bool a4);

    //0x004C0250
    //virtual
    void _spawn(event *a2, entity_base_vhandle a3);
};
