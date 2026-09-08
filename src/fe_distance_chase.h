#pragma once
#include "float.hpp"

struct PanelFile;
struct PanelQuad;

struct fe_distance_chase {
    PanelFile *panels[7];
    PanelQuad *field_1C;
    PanelQuad *field_20;
    int field_24;
    int field_28;
    float field_2C[4];
    float field_3C[4];
    float field_4C[4];
    float field_5C[4];
    bool field_6C;

    //0x006437B0
    void Init(int type_id, const char *a3);

    fe_distance_chase();
    // 0x0062FDE0
    void Update(Float time_inc);

    void DeInit(int a2);
};
