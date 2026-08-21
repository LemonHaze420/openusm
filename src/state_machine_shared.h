#pragma once

#include "mash_virtual_base.h"

#include "mvector.h"

struct from_mash_in_place_constructor;
struct mash_info_struct;

namespace als {

struct state;
struct category;
struct transition_group_base;

struct state_machine_shared : mash_virtual_base {
    state_machine_shared();

    state_machine_shared(from_mash_in_place_constructor *a2);

    mVector<state> state_list;
    mVector<category> category_list;
    mVector<transition_group_base> trans_group_list;

    //virtual
    void destruct_mashed_class();

    //virtual
    void _unmash(mash_info_struct *a2, void *a3);

    //virtual
    int _get_virtual_type_enum() const;

    int _get_mash_sizeof() const;

    //virtual
    int get_mash_sizeof() const;
};
}  // namespace als

extern void als_state_machine_shared_patch();
