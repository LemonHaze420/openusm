#pragma once

#include "eligible_pack_token.h"
#include "fixedstring.h"
#include "string_hash.h"

#include <list.hpp>

struct resource_pack_slot;
struct eligible_pack_category;
struct eligible_pack_streamer;

struct eligible_pack {
    fixedstring<8> field_0;
    fixedstring<8> field_20;

    int field_40;
    string_hash field_44;

    eligible_pack_token field_48;
    eligible_pack_category *field_50;
    _std::list<eligible_pack *> field_54;
    _std::list<eligible_pack *> field_60;
    float field_6C;
    int field_70;

    //0x0054CA90
    eligible_pack(const char *a2, const eligible_pack_token &a3, eligible_pack_category *a4);

    //0x0056BAB0
    ~eligible_pack();

    //0x00532130
    bool is_loadable();

    //0x005320B0
    resource_pack_slot *get_resource_pack_slot();

    //0x00542FC0
    void fixup_family(eligible_pack_streamer *a2);

    //0x0053E5A0
    int add_parent(eligible_pack *a3);

    //0x0053E5F0
    int add_child(eligible_pack *a3);
};

extern void eligible_pack_patch();
