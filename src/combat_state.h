#pragma once

#include "base_state.h"
#include "enhanced_state.h"
#include "float.hpp"
#include "vector3d.h"

#include <cstdint>

struct als_inode;
struct anchor_storage_class;
struct combat_inode;
struct from_mash_in_place_constructor;
struct vector3d;
struct entity;
struct event;
struct entity_base_vhandle;

struct combat_state : ai::enhanced_state {
    combat_inode *field_30;
    als_inode *field_34;
    vector3d field_38;
    vector3d field_44;
    int field_50[6];

    vector3d field_68;
    int field_74[47];

    //0x00471EC0
    combat_state(from_mash_in_place_constructor *a2);

    //0x00487500
    //virtual
    bool find_web_hang_spot();

    static const inline string_hash default_id{static_cast<int>(to_hash("COMBAT"))};
};


//0x004474B0
extern void web_start_call_back(event *a1, entity_base_vhandle a2, void *a3);

extern void combat_state_patch();
