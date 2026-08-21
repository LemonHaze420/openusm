#include "pause_menu_save_load_hookup.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_OFFSET(pause_menu_save_load_hookup, field_24, 0x24);

void pause_menu_save_load_hookup::SaveFile(int a2)
{
    THISCALL(0x0062C970, this, a2);
}

void pause_menu_save_load_hookup::set_current_state(int a2)
{
    THISCALL(0x0062BF90, this, a2);
}

void pause_menu_save_load_hookup::LoadFile(int a2)
{
    THISCALL(0x0062C9B0, this, a2);
}
