#include "main_menu_credits.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(main_menu_credits, 0x48);

main_menu_credits::main_menu_credits(FEMenuSystem *menu_system, int x, int y)
    : FEMenu(menu_system, 0, x, y, 8, 0)
{
    if constexpr (STANDALONE_SYSTEM)
    {
        m_vtbl = 0x00894858;
        field_2C.field_0 = false;
        field_2C.field_8 = nullptr;
        field_2C.field_10 = 0;
        field_2C.field_14 = 0;
        a2 = menu_system;
    }
    else
    {
        THISCALL(0x00625360, this, menu_system, x, y);
    }
}
