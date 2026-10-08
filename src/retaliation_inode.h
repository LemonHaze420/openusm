#pragma once

#include "info_node.h"

namespace ai {

struct mashed_state;

struct retaliation_inode : info_node {
    int counters[2];
    float last_health[2];
    float thresholds[2];

    explicit retaliation_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_core *core);
    void _reset();
    void configure(const mashed_state *state);
    void record_retaliation(int index);
    bool calc_damage_since_last_retaliation(int index) const;
};

}
