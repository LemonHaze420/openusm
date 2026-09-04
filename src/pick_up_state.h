#pragma once

#include "interaction_state.h"

struct from_mash_in_place_constructor;

namespace ai {

struct interaction_inode;

struct pick_up_state : interaction_state {
    int field_58;
    int field_5C;

    //0x00462DB0
    pick_up_state(from_mash_in_place_constructor *a2);

    //0x0046D8A0
    bool begin_carry();

    //0x004752A0
    //virtual
    int frame_advance(Float a2);

    static const inline string_hash default_id{static_cast<int>(to_hash("pick_up"))};
};

}  // namespace ai

extern void pick_up_state_patch();
