#include "concept_art.h"

#include "common.h"
#include "fe_menu_nav_bar.h"
#include "func_wrapper.h"

VALIDATE_SIZE(concept_art, 0x224u);

concept_art::concept_art(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x00894DE8;
    field_2C[0] = reinterpret_cast<int>(a2);
    field_2C[2] = reinterpret_cast<int>(new menu_nav_bar {});
    for (int i = 9; i <= 29; ++i)
        field_2C[i] = 0;
    field_2C[110] = 0;
    field_2C[111] = 0;
}
