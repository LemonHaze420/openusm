#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "float.hpp"
#include "trigger.h"

#include <list.hpp>

struct resource_key;
struct token_def;
struct token_def_list;

struct wds_token_manager {
    struct active_token {
        token_def *field_0;
        vhandle_type<entity> field_4;
        vhandle_type<trigger> field_8;
        bool field_C;
    };

    token_def_list *tokens;
    bool field_4;
    bool editing;
    float field_8;
    int field_C;
    int token_collected_array;
    bool field_14;
    _std::list<active_token> field_18;

    wds_token_manager();

    //0x005586A0
    void initialize(const resource_key &a2);

    //0x00555B50
    void frame_advance(Float a1);

    void register_region(region *reg);

    void unregister_region(region *reg);

    _std::list<wds_token_manager::active_token>::iterator
    remove_active_token(_std::list<wds_token_manager::active_token>::iterator a3, bool a4, bool a5);

    void sub_54C0C0();
};

extern void wds_token_manager_patch();
