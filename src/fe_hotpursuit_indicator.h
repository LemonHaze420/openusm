#pragma once

struct PanelFile;
struct PanelAnimFile;
struct PanelQuad;

struct fe_hotpursuit_indicator {
    bool field_0;
    bool field_1;
    int field_4;
    PanelFile *field_8;
    PanelAnimFile *field_C;
    PanelQuad *field_10;
    PanelQuad *field_14;
    PanelQuad *field_18;

    fe_hotpursuit_indicator();

    void Init();


    void Draw();

    void DeInit();
};
