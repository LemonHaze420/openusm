#pragma once

#include "state_machine.h"

namespace als {

struct layer_state_machine_shared;

struct layer_state_machine : state_machine {
    layer_state_machine();

    void init(layer_state_machine_shared *a2);

    int get_domain_bitmask() const;
};

}  // namespace als
