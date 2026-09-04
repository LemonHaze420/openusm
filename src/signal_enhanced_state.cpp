#include "signal_enhanced_state.h"

#include "common.h"

namespace ai {

VALIDATE_SIZE(signal_enhanced_state, 0x34);

signal_enhanced_state::signal_enhanced_state(from_mash_in_place_constructor *a2) : enhanced_state(a2) {}
}  // namespace ai
