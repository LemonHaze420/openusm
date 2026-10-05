#pragma once

struct PanelFile;
struct PanelQuad;
struct FEText;
struct PanelAnimFile;

struct medal_award_ui {
    PanelFile *field_0;
    PanelQuad *field_4[6];
    PanelQuad *field_1C[4];
    FEText *field_2C;
    FEText *field_30;
    FEText *field_34;
    PanelAnimFile *field_38;
    int field_3C;
    int field_40;
    int field_44;
    bool field_48;

    medal_award_ui();

    void Init();

    void DeInit();
    void Draw();
};
