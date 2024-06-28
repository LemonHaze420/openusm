#include "state_machine_shared.h"

#include "als_category.h"
#include "als_state.h"
#include "als_transition_group_base.h"
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "vtbl.h"

namespace als {
    VALIDATE_SIZE(state_machine_shared, 0x40);

    state_machine_shared::state_machine_shared()
    {
        THISCALL(0x00444790, this);
    }

    void state_machine_shared::destruct_mashed_class()
    {
        void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
        func(this);
    }

    void state_machine_shared::_unmash(mash_info_struct *a2, void *)
    {
        TRACE("als::state_machine_shared::unmash");

        a2->unmash_class_in_place(this->state_list, this);
        a2->unmash_class_in_place(this->category_list, this);
        a2->unmash_class_in_place(this->trans_group_list, this);
    }

    int state_machine_shared::get_mash_sizeof() const
    {
        int (__fastcall *func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
        return func(this);
    }

} // namespace als


void als_state_machine_shared_patch()
{
    {
        FUNC_ADDRESS(address, &als::state_machine_shared::_unmash);
        set_vfunc(0x0087B8FC, address);
    }
}
