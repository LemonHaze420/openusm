#pragma once

#include "conglomerate_interface.h"

#include "vector3d.h"

struct entity;

struct tentacle_interface : conglomerate_interface
{
    bool field_C;
    char field_D[3];
    vector3d field_10;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;

    void initialize_polytubes();

    void begin_zip(const vector3d &a2);

    void tentacle_zip_event_fired();
};
