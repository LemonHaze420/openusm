#include "concept_art2.h"

#include "common.h"
#include "fe_menu_nav_bar.h"
#include "func_wrapper.h"

VALIDATE_SIZE(concept_art2, 0x114u);

concept_art2::concept_art2(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x00894E98;
    field_2C[0] = reinterpret_cast<int>(a2);
    field_2C[2] = reinterpret_cast<int>(new menu_nav_bar {});
    for (int i = 4; i <= 13; ++i)
        field_2C[i] = 0;
}
