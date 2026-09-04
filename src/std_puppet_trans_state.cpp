#include "std_puppet_trans_state.h"

#include "func_wrapper.h"
#include "common.h"

namespace ai {
VALIDATE_SIZE(std_puppet_trans_state, 0x1C);

std_puppet_trans_state::std_puppet_trans_state()
{
    if constexpr (1) {
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x00438C80, this);
    }
}

void std_puppet_trans_state::_unmash(mash_info_struct *a1, void *a2)
{
    base_state::_unmash(a1, a2);
}
}  // namespace ai
