#include "concept_art2.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(concept_art2, 0x114u);

concept_art2::concept_art2(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x006157F0, this, a2, a3, a4);
}
