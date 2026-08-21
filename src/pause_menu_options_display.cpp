#include "pause_menu_options_display.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(pause_menu_options_display, 0x1A4u);

pause_menu_options_display::pause_menu_options_display(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x0061D970, this, a2, a3, a4);
}

void pause_menu_options_display::_Load()
{
    TRACE("pause_menu_options_display::_Load");
    THISCALL(0x0063BE80, this);
}

void pause_menu_options_display_patch()
{
    {
        FUNC_ADDRESS(address, &pause_menu_options_display::_Load);
        set_vfunc(0x00894160, address);
    }
}
