#include "ltd_edition.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(ltd_edition, 0x120u);

ltd_edition::ltd_edition(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x00614570, this, a2, a3, a4);
}
