#include "ltd_edition.h"

#include "common.h"
#include "fe_menu_nav_bar.h"
#include "func_wrapper.h"

VALIDATE_SIZE(ltd_edition, 0x120u);

ltd_edition::ltd_edition(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x00894A78;
    field_2C[0] = reinterpret_cast<int>(a2);
    field_2C[2] = reinterpret_cast<int>(new menu_nav_bar {});
    for (int i = 3; i <= 16; ++i)
        field_2C[i] = 0;
    for (int i = 55; i <= 58; ++i)
        field_2C[i] = 0;
    reinterpret_cast<unsigned char *>(&field_2C[60])[0] = 0;
}
