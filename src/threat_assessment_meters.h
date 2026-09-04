#pragma once

#include "vector2d.h"
#include "vector3d.h"

struct fe_mini_map_dot;
struct PanelQuad;
struct PanelFile;
struct PanelAnimFile;

struct threat_assessment_meters {
    struct tam_instance {
        bool field_0;
        int field_4;
        int field_8;
        vector3d field_C;
        int field_18;
        fe_mini_map_dot *field_1C;
        bool field_20;
        int field_24;
        int field_28;
    };

    int field_0;
    int field_4;
    float field_8;
    int field_C;
    float field_10;
    PanelFile *field_14;
    PanelQuad *field_18[6];
    PanelAnimFile *field_30;
    vector2d field_34[6];
    tam_instance field_64[4];
    vector2d field_114[4];
    float field_134;
    float field_138[4];
    float field_148[4];
    float field_158[4];
    float field_168[4];
    float field_178[4];
    float field_188[4];
    float field_198[4];
    float field_1A8[4];

    //0x006199E0
    threat_assessment_meters();

    //0x00643400
    void init();
};

extern void threat_assessment_meters_patch();
