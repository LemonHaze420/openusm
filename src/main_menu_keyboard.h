#pragma once

#include "femenu.h"
#include "mstring.h"

#include <vector.hpp>

struct PanelQuad;
struct PanelAnimFile;
struct FEText;
struct main_menu_keyboard : FEMenu {
    PanelQuad *field_2C[24];
    PanelQuad *field_8C[2];
    PanelQuad *field_94[4];
    PanelQuad *field_A4;
    PanelAnimFile *field_A8;
    PanelAnimFile *field_AC;
    PanelAnimFile *field_B0;
    PanelAnimFile *field_B4;
    PanelAnimFile *field_B8;
    PanelAnimFile *field_BC;
    FEText *field_C0;
    FEText *field_C4[8];
    mString field_E4;
    _std::vector<char> field_F4;
    int field_104[8];
    FEText *field_124;
    FEText *field_128[4];
    int16_t field_138;
    bool field_13A;
    bool field_13B;
    bool field_13C;
    bool field_13D;
    char field_13E[2];
    FEMenuSystem *field_140;
    int field_144;

    main_menu_keyboard(FEMenuSystem *a2, int a3, int a4);

    void _Init();
    void Draw();
    void Update(Float delta_time);
    void OnUp(int controller);
    void OnDown(int controller);
    void OnLeft(int controller);
    void OnRight(int controller);
    void OnCross(int controller);
    void OnTriangle(int controller);
    void OnSquare(int controller);
    void OnCircle(int controller);
    void update_letter();
    void move_letter(int delta);

    //0x006241E0
    /* virtual */ void OnActivate();

    //0x00613D10
    /* virtual */ void OnDeactivate(FEMenu *a2);
};

extern void main_menu_keyboard_patch();
