#pragma once

#include "sound_interface.h"

struct entity_base;

struct pfx_interface : sound_interface {
    void *field_28;
    entity_base *field_2C;
    int field_30;
    int field_34;
    int field_38;
    int field_3C;

    pfx_interface();

    //virtual
    void release_ifc();
};
