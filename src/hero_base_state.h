#pragma once

#include "base_state.h"
#include "string_hash.h"

namespace ai {

struct hero_base_state : base_state {
    hero_base_state();

    //0x0044D190
    hero_base_state(int a2);

    //0x00474040
    void combat_inode_transition_notification(Float a2, string_hash a3);

    /* virtual */ string_hash get_desired_state_id(Float);

    //0x00478D80
    //virtual
    state_trans_action check_transition(Float a3);

    static inline string_hash NO_TRANS{int(to_hash("DO NOT TRANSITION!"))};
};

}  // namespace ai


extern void hero_base_state_patch();
