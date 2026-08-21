#include "main_menu_keyboard.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

VALIDATE_SIZE(main_menu_keyboard, 0x148);

main_menu_keyboard::main_menu_keyboard(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    THISCALL(0x00633200, this, a2, a3, a4);
}

void main_menu_keyboard::OnActivate()
{
    THISCALL(0x006241E0, this);
}

void main_menu_keyboard::OnDeactivate(FEMenu *a2)
{
    THISCALL(0x00613D10, this, a2);
}

void main_menu_keyboard_patch()
{
    FUNC_ADDRESS(address, &main_menu_keyboard::OnActivate);
    set_vfunc(0x008957BC, address);
}
