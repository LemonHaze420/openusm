#include "pole_swing_state.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "utility.h"

namespace ai {

VALIDATE_SIZE(pole_swing_state, 0x70);

pole_swing_state::pole_swing_state(from_mash_in_place_constructor *a2)
{
    THISCALL(0x0044C4B0, this, a2);
}

void pole_swing_state::activate(ai_state_machine *a2, const mashed_state *a3, const mashed_state *a4,
                                const param_block *a5, base_state::activate_flag_e a6)
{
    sp_log("pole_swing_state::activate:");

    THISCALL(0x0046BD90, this, a2, a3, a4, a5, a6);
}

}  // namespace ai

void pole_swing_state_patch()
{
    {
        FUNC_ADDRESS(address, &ai::pole_swing_state::activate);
        set_vfunc(0x008771B8, address);
    }
}
