#include "main_menu_legal.h"

#include "common.h"

VALIDATE_SIZE(main_menu_legal, 0x38);

main_menu_legal::main_menu_legal(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    this->field_34 = a2;
}
