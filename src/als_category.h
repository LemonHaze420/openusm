#pragma once

#include "als_request_data.h"
#include "mash_virtual_base.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace ai
{
    struct param_block;
}

namespace als
{
    struct animation_logic_system;
    struct state_machine;

    struct category : mash_virtual_base
    {
        string_hash field_4;
        int field_8;
        ai::param_block *field_C;

        category();

        category(from_mash_in_place_constructor *a2);

        bool is_flag_set(uint32_t a2) const
        {
            return ((1 << a2) & static_cast<uint16_t>(this->field_8)) != 0;
        }

        //virtual
        request_data do_implicit_trans(
            animation_logic_system *a4,
            state_machine *a5);

        //virtual
        request_data do_incoming_trans(
            animation_logic_system *a3,
            state_machine *a4);

        //
        void _unmash(mash_info_struct *, void *);

        int get_mash_sizeof() const;
    };
}

extern void als_category_patch();
