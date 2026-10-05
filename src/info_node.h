#pragma once

#include "mash.h"
#include "mash_virtual_base.h"
#include "param_block.h"
#include "string_hash.h"

struct actor;
struct from_mash_in_place_constructor;

namespace ai {

struct ai_core;

struct info_node : mash_virtual_base {
    string_hash field_4;
    ai_core *field_8;
    actor *field_C;
    param_block my_param_block;

    static void *native_vtable();

    //0x006D6F20
    info_node();

    //0x006D9930
    info_node(from_mash_in_place_constructor *a2);

    void initialize(mash::allocation_scope a2);

    auto get_name() const
    {
        return this->field_4;
    }

    actor *get_actor() const
    {
        return this->field_C;
    }

    ai_core *get_core()
    {
        return this->field_8;
    }

    ~info_node() = default;

    //0x006D6FA0
    //virtual
    void _unmash(mash_info_struct *a1, void *a3);

    //virtual
    bool does_need_advance() const;

    //virtual
    void frame_advance(Float a2);

    //virtual
    void activate(ai_core *a2);

    void _activate(ai_core *a2);
    void _reset();
    void _destruct_mashed_class();

    //virtual
    void deactivate();

    //virtual
    void reset();

    //virtual
    int get_mash_sizeof() const;
};

}  // namespace ai


extern void info_node_patch();
