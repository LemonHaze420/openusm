#pragma once
#include "float.hpp"

struct PanelFile;
struct PanelQuad;
struct PanelAnimFile;

struct fe_distance_race {
    PanelFile *field_0;
    PanelQuad *field_4;
    PanelQuad *field_8;
    PanelQuad *field_C;
    PanelQuad *field_10;
    PanelQuad *field_14;
    PanelQuad *field_18;
    PanelQuad *field_1C;
    PanelQuad *field_20;
    PanelQuad *field_24;
    PanelQuad *field_28;
    PanelQuad *field_2C;
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
    PanelAnimFile *field_60;
    int field_64;
    int field_68;
    float field_6C;
    float field_70[4];
    float field_80[4];
    float field_90[4];
    float field_A0[4];
    float field_B0[4];

    bool field_C0;

    //0x0060CED0
    fe_distance_race();

    //0x00643860
    void Init();
    // 0x0062FE40
    void Update(Float time_inc);

    void DeInit();
};
