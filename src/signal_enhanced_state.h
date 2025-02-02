#pragma once

#include "enhanced_state.h"

namespace ai {

struct signal_enhanced_state : enhanced_state {
    bool field_30;

    signal_enhanced_state(from_mash_in_place_constructor *a2);
};
} // namespace ai
