#include "main_menu_credits.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(main_menu_credits, 0x48);

main_menu_credits::main_menu_credits(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x00625360, this, a2, a3, a4);
}
