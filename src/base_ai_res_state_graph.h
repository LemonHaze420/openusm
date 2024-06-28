#pragma once

#include "mash.h"
#include "mvector.h"
#include "resource_key.h"

struct from_mash_in_place_constructor;

namespace ai {
struct mashed_state;
struct base_state;

struct state_graph {
    resource_key field_0;
    mVector<mashed_state> my_states;
    mashed_state *field_1C;
    mVector<base_state> field_20;

    //0x006DA190
    state_graph(from_mash_in_place_constructor *);

    void initialize(mash::allocation_scope scope);

    mashed_state * get_initial_state() const {
        return this->field_1C;
    }

    void destruct_mashed_class();

    void unmash(mash_info_struct *a1, void *a3);

    unsigned int get_size_memory_block() const;

    resource_key get_name() const {
        return this->field_0;
    }

    //0x006D8480
    mashed_state *find_state(string_hash a2) const;
};

} // namespace ai

extern void state_graph_patch();
