#pragma once

#include "femenu.h"

struct FEMultiLineText;

struct main_menu_legal : FEMenu {
    FEMultiLineText *field_2C;
    float field_30;
    FEMenuSystem *field_34;

    main_menu_legal(FEMenuSystem *a2, int a3, int a4);

    void Update(Float delta_time);
    void Draw();
    void OnActivate();
    void OnDeactivate();
};
