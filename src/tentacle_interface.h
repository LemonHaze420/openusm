#pragma once

#include "conglomerate_interface.h"

#include "mashable_vector.h"
#include "vector3d.h"

#include <cstddef>
#include <cstdint>

struct entity;

// Retail vector loader 0x004D16E0: 0x4C-byte records, with nested
// eight-byte records at 0x3C and entity pointers at 0x44.
struct tentacle_info {
    std::uint8_t field_0[0x3C];
    mashable_vector<std::uint64_t> field_3C;
    mashable_vector<entity *> field_44;
};

static_assert(sizeof(tentacle_info) == 0x4C);
static_assert(offsetof(tentacle_info, field_3C) == 0x3C);
static_assert(offsetof(tentacle_info, field_44) == 0x44);

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
