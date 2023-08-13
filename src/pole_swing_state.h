#pragma once

#include "enhanced_state.h"
#include "vector3d.h"

struct from_mash_in_place_constructor;

namespace ai {

struct pole_swing_state : enhanced_state {
    vector3d field_30;
    vector3d field_3C;
    vector3d field_48;
    int field_54[7];

    //0x0044C4B0
    pole_swing_state(from_mash_in_place_constructor *);

    //0x0046BD90
    //virtual
    void activate(ai_state_machine *a2,
                  const mashed_state *a3,
                  const mashed_state *a4,
                  const param_block *a5,
                  base_state::activate_flag_e a6);

    static const inline string_hash default_id{to_hash("POLE_SWING")};
};

} // namespace ai

extern void pole_swing_state_patch();
