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
        if constexpr (1) {
            static void * g_vtbl[] = {
                nullptr,
                func_address(&_unmash),
                nullptr,
                func_address(&_get_virtual_type_enum),
                nullptr,
                func_address(&_is_or_is_subclass_of),
                nullptr,
                func_address(&_get_mash_sizeof),
                nullptr
            };

            this->m_vtbl = CAST(m_vtbl, &g_vtbl);
        } else {
            THISCALL(0x00444790, this);
        }
    }


    state_machine_shared::state_machine_shared(from_mash_in_place_constructor * a2) : state_list(a2), category_list(a2), trans_group_list(a2)
    {
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

    int state_machine_shared::_get_virtual_type_enum() const {
        return 484;
    }

    int state_machine_shared::_get_mash_sizeof() const
    {
        return sizeof(state_machine_shared);
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
