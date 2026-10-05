#pragma once

#include "femenu.h"

#include "mstring.h"

struct FEMenuSystem;
struct FEText;
struct PanelFile;
struct PanelAnimFile;
struct PanelQuad;

struct fe_dialog_text : FEMenu {
    PanelFile *panel;
    PanelQuad *field_30;
    PanelQuad *field_34;
    PanelQuad *field_38;
    PanelQuad *field_3C;
    PanelQuad *field_40;
    PanelQuad *field_44;
    PanelQuad *field_48;
    PanelQuad *field_4C;
    PanelQuad *field_50;
    PanelQuad *field_54;
    PanelQuad *field_58;
    PanelQuad *field_5C;
    PanelQuad *field_60;
    PanelQuad *field_64;
    PanelQuad *field_68;
    PanelQuad *field_6C;
    PanelQuad *field_70;
    PanelQuad *field_74;

    FEText *field_78;
    FEText *field_7C;
    FEText *field_80;
    FEText *field_84;
    FEText *field_88;
    FEText *field_8C;
    PanelAnimFile *field_90;
    PanelAnimFile *field_94;
    int field_98;
    int field_9C;
    bool field_A0;
    bool field_A1;
    bool field_A2;
    bool field_A3;
    int field_A4;
    float field_A8[4];
    float field_B8[4];
    float field_C8[4];
    float field_D8[4];
    float field_E8;
    float field_EC[2];
    float field_F4[2];
    int field_FC;
    float field_100;
    float field_104;
    int field_108;

    //0x0060D570
    fe_dialog_text(FEMenuSystem *a2, int a3, int a4);

    struct string {
        int field_0[2];
        char *data;
        int field_C;
    };

    //0x0060D960
    void set_text(string a1);

    //0x0060DB30
    void set_title(string a2);

    //0x00643C90
    //virtual
    void _Load();
    void Draw();
    void Update(Float elapsed);
    void OnActivate();
    void OnDeactivate();
    void OnCross(int controller);
    void OnTriangle(int controller);
    void OnLeft(int controller);
    void OnRight(int controller);
    void OnUp(int controller);
    void OnDown(int controller);
    void OnAnyButtonRelease(int controller, int button);
    void set_yes_no(bool enabled);
    int get_result() const;
};

extern void fe_dialog_text_patch();
