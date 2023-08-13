#include "combo_system.h"
#include "common.h"
#include "mash_info_struct.h"
#include "trace.h"
#include "vtbl.h"

VALIDATE_SIZE(combo_system_chain, 0x44);
VALIDATE_SIZE(combo_system_chain::telegraph_info, 0xC);
VALIDATE_SIZE(combo_system, 0x50);


void combo_system_chain::telegraph_info::_unmash(mash_info_struct *a1, void *a2)
{
    TRACE("combo_system_chain::telegraph_info::unmash");

    ;
}


int combo_system_chain::telegraph_info::get_mash_sizeof()
{
    auto &func = get_vfunc(m_vtbl, 0x18);
    return func(this);
}

void combo_system_chain::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("combo_system_chain::unmash");

    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_14, this);
    a1->unmash_class_in_place(this->field_1C, this);
}

combo_system::combo_system() {}

combo_system_weapon *combo_system::get_weapon(int idx) {
    return this->field_28.m_data[(uint16_t) idx];
}

int combo_system::get_num_weapons() {
    return this->field_28.size();
}

void combo_system::unmash(mash_info_struct *a1, void *a3)
{
    TRACE("combo_system::unmash");

    a1->unmash_class_in_place(this->field_0, this);

    a1->unmash_class_in_place(this->field_14, this);

    a1->unmash_class_in_place(this->field_28, this);

    a1->unmash_class_in_place(this->field_3C, this);
}

void combo_system_patch()
{
    {
        FUNC_ADDRESS(address, &combo_system_chain::telegraph_info::_unmash);
        set_vfunc(0x008737C4, address);
    }
}
