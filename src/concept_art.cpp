#include "concept_art.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(concept_art, 0x224u);

concept_art::concept_art(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x00615420, this, a2, a3, a4);
}
