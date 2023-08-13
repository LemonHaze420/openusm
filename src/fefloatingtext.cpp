#include "fefloatingtext.h"

#include "common.h"
#include "utility.h"
#include "vtbl.h"
#include "func_wrapper.h"

VALIDATE_SIZE(FEFloatingText, 0x84);

FEFloatingText::FEFloatingText()
{
    THISCALL(0x0043C4A0, this);
}

void FEFloatingText::_unmash(mash_info_struct *a1, void *a3)
{
    FEText::unmash(a1, this);
}

int FEFloatingText::_get_mash_sizeof()
{
#ifdef TARGET_XBOX
    return 0x7C;
#else
    return 0x84;
#endif
}

void FEFloatingText_patch()
{
    {
        FUNC_ADDRESS(address, &FEFloatingText::_unmash);
        set_vfunc(0x0087A0F4, address);
    }

    {
        FUNC_ADDRESS(address, &FEFloatingText::_get_mash_sizeof);
        set_vfunc(0x0087A13C, address);
    }
}
