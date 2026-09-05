#include "landmarks.h"

#include "common.h"
#include "fe_menu_nav_bar.h"
#include "func_wrapper.h"

VALIDATE_SIZE(landmarks, 0x1F4u);

landmarks::landmarks(FEMenuSystem *a2, int a3, int a4) : FEMenu(a2, 0, a3, a4, 8, 0) {
    m_vtbl = 0x00894C08;
    field_2C[0] = reinterpret_cast<int>(a2);
    field_2C[2] = reinterpret_cast<int>(new menu_nav_bar {});
    for (int i = 0; i < 15; ++i)
        reinterpret_cast<unsigned char *>(&field_2C[39 + 4 * i])[0] = 0;
}
