#pragma once

#include "color32.h"
#include "femenu.h"
#include "mstring.h"

struct game_data_essentials;
struct FEText;
struct PanelAnimFile;
struct PanelQuad;

struct main_menu_load : FEMenu {
    struct save_slot {
        mString title;
        mString date;
        bool disabled;
        char padding[3];
    };

    float field_2C;
    float field_30;
    float field_34;
    color32 field_38;
    color32 field_3C;
    PanelQuad *field_40[30];
    PanelAnimFile *field_B8;
    PanelAnimFile *field_BC;
    FEText *field_C0[7];
    int field_DC;
    int field_E0;
    mString field_E4;
    mString field_F4;
    save_slot field_104[3];
    FEMenuSystem *field_170;

    main_menu_load(FEMenuSystem *a2, int a4, int a5);

    void _Init();

    void SetSaveSlot(int slot, const game_data_essentials *data);
};
