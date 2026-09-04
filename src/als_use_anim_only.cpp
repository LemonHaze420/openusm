#include "als_use_anim_only.h"

#include "common.h"
#include "vtbl.h"

namespace als {

VALIDATE_SIZE(use_anim_only, 0x14);

void use_anim_only::post_anim_action(Float arg0)
{
    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x24));
    func(this, nullptr, arg0);
}

}  // namespace als
