#include "launch_layer_state.h"

#include "common.h"
#include "mashed_state.h"

namespace ai {

VALIDATE_SIZE(launch_layer_state, 0x44);

launch_layer_state::launch_layer_state(from_mash_in_place_constructor *a2) : signal_enhanced_state(a2) {}

resource_key launch_layer_state::get_layer_resource_key()
{
    auto *v2 = this->my_mashed_state->field_0.get_pb_fixedstring(layer_to_launch_hash());

    string_hash v6{v2};

    resource_key a2;
    a2.set(v6, RESOURCE_KEY_TYPE_FX_CACHE);

    return a2;
}

}  // namespace ai
