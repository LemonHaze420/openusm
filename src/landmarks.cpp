#include "landmarks.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(landmarks, 0x1F4u);

landmarks::landmarks(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x006260F0, this, a2, a3, a4);
}
