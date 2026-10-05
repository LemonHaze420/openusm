#pragma once

#include "conglomerate_interface.h"

#include "mashable_vector.h"
#include "color32.h"
#include "float.hpp"
#include "string_hash.h"
#include "vector3d.h"

#include <cstddef>
#include <cstdint>

struct entity;

struct alignas(8) tentacle_control_id {
    string_hash id;
    uint32_t field_4;
};

struct tentacle_info {
    color32 color;
    uint32_t tentacle_id;
    uint32_t field_8;
    float radius;
    char texture_name[32];
    string_hash zip_entity_id;
    uint32_t field_34;
    entity_base *zip_entity;
    mashable_vector<tentacle_control_id> field_3C;
    mashable_vector<entity_base *> field_44;
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
    float field_2C;
    float field_30;
    int field_34;

    void initialize_polytubes();

    void render(Float fade);
    void standard_tentacle_update(int index, tentacle_info &info);
    void update_tentacle_zip_aiming(Float elapsed, int index);

    void begin_zip(const vector3d &a2);

    void tentacle_zip_event_fired();

    void cancel_zip();

    //virtual
    void release_ifc();
};
