#include "als_transition_group_base.h"

#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include "common.h"

namespace als
{
    VALIDATE_SIZE(transition_group_base, 0x4);

    transition_group_base::transition_group_base() {
    }

    void transition_group_base::_unmash(mash_info_struct *, void *)
    {
        ;
    }

    int transition_group_base::get_mash_sizeof() const
    {
        int (__fastcall *func)(const transition_group_base *) = CAST(func, get_vfunc(m_vtbl, 0x1C));
        return func(this);
    }
}

void als_transition_group_base_patch()
{
}
