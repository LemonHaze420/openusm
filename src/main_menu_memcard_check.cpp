#include "main_menu_memcard_check.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(main_menu_memcard_check, 0x1A0u);
VALIDATE_OFFSET(main_menu_memcard_check, field_100, 0x100);

main_menu_memcard_check::main_menu_memcard_check(FEMenuSystem *a2, int a4, int a5) : FEMenu(a2, 0, a4, a5, 8, 0)
{
    THISCALL(0x00632B30, this, a2, a4, a5);
}
