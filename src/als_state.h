#pragma once

#include <cstdint>

#include "als_request_data.h"
#include "mash_virtual_base.h"
#include "string_hash.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace ai {
struct param_block;
}

namespace als {

struct animation_logic_system;
struct state_machine;

enum state_flags {};

struct state : mash_virtual_base {
    string_hash m_state_id;
    string_hash m_cat_id;
    uint16_t field_C;
    uint16_t field_E;
    ai::param_block *field_10;

    state();

    state(from_mash_in_place_constructor *a2);

    string_hash get_state_id() const
    {
        return m_state_id;
    }

    string_hash get_category_id() const
    {
        return m_cat_id;
    }

    bool is_flag_set(state_flags a2) const
    {
        return static_cast<uint16_t>(a2 & this->field_C) != 0;
    }

    void _unmash(mash_info_struct *, void *);

    request_data do_implicit_trans(animation_logic_system *a4, state_machine *a5);

    //virtual
    string_hash get_nal_anim_name() const;

    int _get_mash_sizeof() const;

    //virtual
    int get_mash_sizeof() const;
};

}  // namespace als

extern void als_state_patch();
