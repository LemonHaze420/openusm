#include "main_menu_load.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(main_menu_load, 0x174);

main_menu_load::main_menu_load(FEMenuSystem *a2, int a4, int a5) : FEMenu(a2, 0, a4, a5, 8, 0)
{
    THISCALL(0x00623950, this, a2, a4, a5);
}
