#include "pause_menu_save_load_display.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(pause_menu_save_load_display, 0xF4u);

pause_menu_save_load_display::pause_menu_save_load_display(FEMenuSystem *a2, int a3, int a4)
    : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x0062B820, this, a2, a3, a4);
}

void pause_menu_save_load_display::_Load()
{
    TRACE("pause_menu_save_load_display::_Load");
    THISCALL(0x0063D080, this);
}

void pause_menu_save_load_display_patch()
{
    {
        FUNC_ADDRESS(address, &pause_menu_save_load_display::_Load);
        set_vfunc(0x008942C0, address);
    }
}
