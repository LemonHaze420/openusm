#pragma once

#include "femenu.h"

struct main_menu_start : FEMenu {
    int field_2C[61];

    float field_120;
    float field_124;
    bool field_128;
    bool field_129;
    uint16_t field_12A;

    FEMenuSystem *field_12C;

    main_menu_start(FEMenuSystem *a2, int a3, int a4);

    void _Init();
    void _OnActivate();
    void _OnDeactivate();
    void OnStart(int controller);
    void OnCross(int controller);
    bool IsIdle() const;
    void Update(Float delta_time);
};
