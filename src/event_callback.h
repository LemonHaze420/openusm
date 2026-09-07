#pragma once

#include "utility.h"

struct event_callback {
    int m_vtbl;
    void *field_4;
    int id;
    bool field_C;
    bool field_D;

    event_callback(void *a2, bool a3);

    void _finalize(bool a2);

    void *operator new(size_t size);

    void operator delete(void *ptr, size_t size);

#if STANDALONE_SYSTEM
    static inline int id_counter_storage = 0;
    static inline int &id_counter = id_counter_storage;
#else
    static inline int &id_counter = *bit_cast<int *>(0x0095A6D8);
#endif
};
