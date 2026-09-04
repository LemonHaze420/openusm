#include "main_menu_start.h"

#include "common.h"

VALIDATE_SIZE(main_menu_start, 0x130);

main_menu_start::main_menu_start(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0)
{
    this->field_128 = 0;
    this->field_12C = a2;
    this->field_120 = 0.0;
    this->field_124 = 0.0;
    this->field_12A = 0;
    this->field_128 = 0;
}
