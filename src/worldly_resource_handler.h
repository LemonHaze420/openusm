#pragma once

#include "progress.h"

#include <cstdint>

struct worldly_pack_slot;
struct limited_timer;

struct worldly_resource_handler {
    enum eBehavior {
        LOAD = 0,
        UNLOAD = 1,
    };

    std::intptr_t m_vtbl;
    progress field_4;
    worldly_pack_slot *my_slot;
    int field_C;

    void *operator new(std::size_t sz);

    void operator delete(void *ptr, std::size_t sz);

    void finalize(bool a2);

    /* virtual */ ~worldly_resource_handler() = default;

    /* virtual */ bool handle(eBehavior behavior, limited_timer *a5) /* = 0 */;
};
