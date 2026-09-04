#include "pick_up_state.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

namespace ai {

VALIDATE_SIZE(pick_up_state, 0x60);

pick_up_state::pick_up_state(from_mash_in_place_constructor *a2) : interaction_state(a2)
{
    this->field_30 = nullptr;
    this->field_50 = 0;
    this->field_54 = 0;

    this->field_5C = 0;
}

int pick_up_state::frame_advance(Float a2)
{
    auto v3 = ai::interaction_state::frame_advance(a2);
    if (v3 == 1) {
        this->begin_carry();
    }

    return v3;
}

bool pick_up_state::begin_carry()
{
    sp_log("pick_up_state::begin_carry:");

    return (bool)THISCALL(0x0046D8A0, this);
}

}  // namespace ai

void pick_up_state_patch()
{
    {
        FUNC_ADDRESS(address, &ai::pick_up_state::begin_carry);
        REDIRECT(0x004752B7, address);
        REDIRECT(0x00479417, address);
        REDIRECT(0x004857CF, address);
    }
}
