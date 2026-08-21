#include "combat_state.h"

#include "actor.h"
#include "anchor_storage_class.h"
#include "common.h"
#include "entity_base_vhandle.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "utility.h"
#include "variable.h"

VALIDATE_SIZE(combat_state, 0x130);

combat_state::combat_state(from_mash_in_place_constructor *a2)
{
    THISCALL(0x00471EC0, this, a2);
}

void web_start_call_back(event *a1, entity_base_vhandle a2, void *a3)
{
    CDECL_CALL(0x004474B0, a1, a2, a3);
}

bool combat_state::find_web_hang_spot()
{
    sp_log("combat_state::find_web_hang_spot:");
    if constexpr (1) {
        auto *act = this->get_actor();

        anchor_storage_class v8 = ai_find_best_pole(act, YVEC, 10.0, 20.0, 20.0, 0.0);
        if (v8.field_0.get_volatile_ptr() == nullptr) {
            return false;
        }

        this->field_68 = v8.get_target();

        auto v9 = this->field_68 - act->get_abs_position();

        this->field_38 = v9.normalized();
        return true;
    } else {
        return (bool)THISCALL(0x00487500, this);
    }
}

anchor_storage_class ai_find_best_pole(entity *arg4, const vector3d &arg8, Float a3, Float a5, Float a6, Float a7)
{
    anchor_storage_class result;
    CDECL_CALL(0x00486EE0, &result, arg4, &arg8, a3, a5, a6, a7);

    return result;
}

void combat_state_patch()
{
    {
        FUNC_ADDRESS(address, &combat_state::find_web_hang_spot);
        set_vfunc(0x0087B090 + 0x54, address);
    }
}
