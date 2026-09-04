#pragma once

#include "conglomerate_interface.h"

#include "mashable_vector.h"
#include "vector3d.h"

struct entity;

struct tentacle_info {};

struct tentacle_interface : conglomerate_interface {
    bool field_C;
    char field_D[3];
    vector3d field_10;
    mashable_vector<tentacle_info> field_1C;
    entity **field_24;
    int field_28;
    int field_2C;
    int field_30;
    int field_34;

    void initialize_polytubes();

    void begin_zip(const vector3d &a2);

    void tentacle_zip_event_fired();

    void cancel_zip();

    //virtual
    void release_ifc();
};
