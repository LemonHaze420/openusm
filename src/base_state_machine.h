#pragma once

#include "state_machine.h"

namespace als {

struct state_machine_shared;

struct base_state_machine : state_machine {
    base_state_machine();

    //0x004995D0
    void init(state_machine_shared *a2);

    bool is_curr_state_biped_physics() const;
};

}  // namespace als
