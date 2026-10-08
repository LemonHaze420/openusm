#include "combat_state.h"

#include "actor.h"
#include "ai_find_best_swing_anchor.h"
#include "anchor_storage_class.h"
#include "common.h"
#include "entity_base_vhandle.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "utility.h"
#include "variable.h"

VALIDATE_SIZE(combat_state, 0x130);

combat_state::combat_state()
    : ai::enhanced_state(), field_30{}, field_34{}, field_38{}, field_44{}, facing{}, movement_distance{}, field_60{},
      field_64{}, field_68{}, field_74{}, field_78{}, field_7C{}, field_80{}, field_84{}, field_88{}, padding_89{},
      field_8C{}, field_90{}, frame_function{}, activate_function{}, deactivate_function{}, field_A0{}, field_B8{},
      field_BC{}, field_C0{}, field_C4{}, field_D4{}, field_E4{}, field_F4{}, field_F8{}, field_F9{}, field_FA{},
      field_FB{}, field_FC{}, field_FD{}, field_FE{}, field_FF{}, callback_ids{}
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
    field_C4.m_data = nullptr;
    field_C4.m_max_size = 0;
    field_D4.m_data = nullptr;
    field_D4.m_max_size = 0;
    field_E4.m_data = nullptr;
    field_E4.m_max_size = 0;
}

combat_state::combat_state(from_mash_in_place_constructor *a2)
    : ai::enhanced_state(a2), field_A0(a2), field_B8{0}, field_C4(a2), field_D4(a2), field_E4(a2), field_F4{0}
{
    m_vtbl = reinterpret_cast<int>(native_vtable());
}


bool combat_state::find_web_hang_spot()
{
    auto *act = get_actor();
    anchor_storage_class anchor = ai_find_best_pole(act, YVEC, 10.0, 20.0, 20.0, 0.0);
    if (anchor.field_0.get_volatile_ptr() == nullptr)
        return false;
    field_68 = anchor.get_target();
    field_38 = (field_68 - act->get_abs_position()).normalized();
    return true;
}

void combat_state_patch()
{
    {
        FUNC_ADDRESS(address, &combat_state::find_web_hang_spot);
        set_vfunc(0x0087B090 + 0x54, address);
    }
}
