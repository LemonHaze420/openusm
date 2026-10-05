#pragma once

#include "conglomerate_interface.h"

#include "float.hpp"
#include "mashable_vector.h"
#include "variable.h"
#include "string_hash.h"

#include <cstdint>

#include <vector.hpp>

struct conglomerate;
struct entity;
struct terrain_decal;

struct alignas(8) decal_hash_record {
    string_hash hash;
    std::uint32_t field_4;
};

static_assert(sizeof(decal_hash_record) == 8);

struct decal_data_interface : conglomerate_interface {
    bool field_C;
    bool field_D;
    conglomerate *field_10;
    _std::vector<entity *> *field_14;
    int field_18;
    int field_1C;
    mashable_vector<decal_hash_record> field_20;
    mashable_vector<decal_hash_record> field_28;
    mashable_vector<float> field_30;
    mashable_vector<terrain_decal> field_38[9];

    decal_data_interface(conglomerate *a2);

    bool is_dynamic() const;

    void add_to_decal_ifc_list();

    void remove_from_decal_ifc_list();

    //0x004D1CC0
    static void frame_advance_all_decal_interfaces(Float a1);

    //virtual
    void release_ifc();

    //virtual
    void constructor_common();

    //virtual
    void destructor_common();

    static inline auto &all_decal_interfaces = var<_std::vector<decal_data_interface *> *>(0x0095A6BC);
};
