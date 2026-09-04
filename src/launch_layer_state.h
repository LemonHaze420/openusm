#pragma once

#include "signal_enhanced_state.h"

#include "resource_key.h"

namespace ai {
struct launch_layer_state : signal_enhanced_state {
    resource_key field_34;
    int field_3C;
    int field_40;

    launch_layer_state(from_mash_in_place_constructor *a2);

    //virtual
    [[nodiscard]] resource_key get_layer_resource_key();

    static inline Var<string_hash> layer_to_launch_hash{0x0096C0E4};
};
}  // namespace ai
