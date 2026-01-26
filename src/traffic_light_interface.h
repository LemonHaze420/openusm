#pragma once

#include "actor_interface.h"

struct traffic_light_interface : actor_interface {
    int field_C;

    traffic_light_interface(actor *a2);

    void * operator new(std::size_t sz);

    void operator delete(void *ptr, std::size_t sz);
};
