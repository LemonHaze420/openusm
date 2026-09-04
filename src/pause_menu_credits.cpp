#include "pause_menu_credits.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(pause_menu_credits, 0x48u);

pause_menu_credits::pause_menu_credits(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    this->field_2C.field_10 = 0;
    this->field_2C.field_8 = nullptr;
    this->field_2C.field_0 = 0;
    this->field_2C.field_14 = 0;
    this->field_44 = a2;
}
