#pragma once

#include "entity_base_vhandle.h"
#include "utility.h"

struct event;

struct event_callback {
    int m_vtbl;
    void *field_4;
    int id;
    bool field_C;
    bool field_D;

    event_callback(void *a2, bool a3);

    //virtual
    void finalize(bool a2);

    void *operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    bool is_disabled() const
    {
        return this->field_C;
    }

    int get_id() const
    {
        return this->id;
    }

    void spawn(event *a2, entity_base_vhandle a3);

#if STANDALONE_SYSTEM
    static inline int id_counter_storage = 0;
    static inline int &id_counter = id_counter_storage;
#else
    static inline int &id_counter = *bit_cast<int *>(0x0095A6D8);
#endif
};
