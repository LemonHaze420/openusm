#include "covers.h"

#include "common.h"
#include "fe_menu_nav_bar.h"
#include "func_wrapper.h"

VALIDATE_SIZE(covers, 0x5E0u);

covers::covers(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x00894CB8;
    field_2C[0] = reinterpret_cast<int>(a2);
    field_2C[2] = reinterpret_cast<int>(new menu_nav_bar {});

    for (int i = 7; i <= 26; ++i)
        field_2C[i] = 0;
    for (int i = 0; i < 79; ++i)
        reinterpret_cast<unsigned char *>(&field_2C[31 + 4 * i])[0] = 0;

    field_2C[362] = 20;
    field_2C[363] = 40;
    field_2C[364] = 60;
}
