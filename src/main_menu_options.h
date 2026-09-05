#pragma once

#include "color32.h"
#include "femenu.h"

struct PanelAnimFile;
struct PanelQuad;

struct main_menu_options : FEMenu {
    float field_2C;
    float field_30;
    float field_34;
    color32 field_38;
    color32 field_3C;
    PanelQuad *field_40[24];

    PanelQuad *field_A0;
    PanelQuad *field_A4;
    PanelQuad *field_A8;
    PanelQuad *field_AC;
    PanelQuad *field_B0;
    PanelQuad *field_B4;
    PanelQuad *field_B8[2];
    PanelQuad *field_C0[4];
    FEText *field_D0;
    PanelAnimFile *field_D4;
    PanelAnimFile *field_D8;
    PanelAnimFile *field_DC;
    PanelAnimFile *field_E0;
    PanelAnimFile *field_E4;
    PanelAnimFile *field_E8;
    FEText *field_EC[6];
    int16_t field_104;
    int16_t field_106;
    bool field_108;
    bool field_109;
    bool field_10A;
    FEMenuSystem *field_10C;

    main_menu_options(FEMenuSystem *a2, int a3, int a4);

    void _Init();

    /* virtual */ void Update(Float a3);

    //0x0062CE60
    /* virtual */ void OnActivate();

    /* virtual */ void OnDown(int a2);

    /* virtual */ void OnCross(int a2);

    void update_highlight();
};

extern void main_menu_options_patch();
