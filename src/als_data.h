#pragma once

namespace als {

struct animation_logic_system;
struct state_machine;

struct als_data {
    animation_logic_system *field_0;
    state_machine *field_4;

    als_data(animation_logic_system *a2, state_machine *a3);
};
}  // namespace als
