#include "covers.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(covers, 0x5E0u);

covers::covers(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x006268B0, this, a2, a3, a4);
}
