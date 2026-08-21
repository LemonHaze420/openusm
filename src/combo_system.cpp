#include "combo_system.h"

#include "combo_system_move.h"
#include "combo_system_weapon.h"
#include "common.h"
#include "mash_info_struct.h"
#include "trace.h"
#include "vtbl.h"

VALIDATE_SIZE(combo_system_chain, 0x44);
VALIDATE_SIZE(combo_system_chain::telegraph_info, 0xC);
VALIDATE_SIZE(combo_system, 0x50);

void combo_system_chain::telegraph_info::_unmash(mash_info_struct *, void *)
{
    TRACE("combo_system_chain::telegraph_info::unmash");

    ;
}

int combo_system_chain::telegraph_info::get_mash_sizeof()
{
    int(__fastcall * func)(combo_system_chain::telegraph_info *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    return func(this);
}

combo_system_chain::combo_system_chain(from_mash_in_place_constructor *a2) : field_0(a2), field_14(a2), field_1C(a2)
{
    this->initialize(mash::FROM_MASH);
}

void combo_system_chain::initialize(mash::allocation_scope a2)
{
    if (a2 == mash::ALLOCATED) {
        this->field_18 = 0;
        this->field_2C = 0;
        this->field_30 = 0x40000000;
        this->field_34 = 0;
        this->field_38 = 3.4028235e38;
        this->field_3C = 1.0;
        this->field_40 = -1.0;
    }
}

void combo_system_chain::unmash(mash_info_struct *a1, void *)
{
    TRACE("combo_system_chain::unmash");

    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_14, this);
    a1->unmash_class_in_place(this->field_1C, this);
}

combo_system::combo_system() {}

combo_system::combo_system(from_mash_in_place_constructor *a2) : field_0(a2), field_14(a2), field_28(a2), field_3C(a2)
{}

combo_system_weapon *combo_system::get_weapon(int idx)
{
    return this->field_28.m_data[(uint16_t)idx];
}

int combo_system::get_num_weapons()
{
    return this->field_28.size();
}

void combo_system::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("combo_system::unmash");

    if constexpr (1) {
        a1->unmash_class_in_place(this->field_0, this);

        a1->unmash_class_in_place(this->field_14, this);

        a1->unmash_class_in_place(this->field_28, this);

        a1->unmash_class_in_place(this->field_3C, this);
    } else {
        THISCALL(0x00489720, this, a1, a3);
    }
}

void combo_system_patch()
{
    {
        FUNC_ADDRESS(address, &combo_system_chain::telegraph_info::_unmash);
        set_vfunc(0x008737C4, address);
    }

    {
        FUNC_ADDRESS(address, &combo_system::unmash);
        REDIRECT(0x006D7265, address);
    }
}
