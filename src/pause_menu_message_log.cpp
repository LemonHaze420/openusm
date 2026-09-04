#include "pause_menu_message_log.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"
#include "trace.h"

VALIDATE_SIZE(pause_menu_message_log, 0x100u);

pause_menu_message_log::pause_menu_message_log(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x00611BD0, this, a2, a3, a4);
}

void pause_menu_message_log::_Load()
{
    TRACE("pause_menu_message_log::Load");

    THISCALL(0x0063D560, this);
}

void pause_menu_message_log_patch()
{
    {
        FUNC_ADDRESS(address, &pause_menu_message_log::_Load);
        set_vfunc(0x0008943E8, address);
    }
}
