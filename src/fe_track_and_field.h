#pragma once

#include "color32.h"
#include "float.hpp"
#include "utility.h"

struct PanelFile;
struct PanelQuad;

struct fe_track_and_field {
    std::intptr_t m_vtbl;
    PanelFile *field_4;
    PanelQuad *field_8[14];
    PanelQuad *field_40;
    PanelQuad *field_44;
    float field_48;
    bool field_4C;
    bool field_4D;
    float field_50;
    float field_54;
    float field_58;
    float field_5C;
    float field_60;
    float field_64;
    color32 field_68;
    color32 field_6C;
    color32 field_70;

    //0x0060E1F0
    fe_track_and_field();

    //0x006440C0
    void _Init();

    //virtual
    void Init();

    //0x00630150
    void _Update(Float a2);

    //virtual
    void Update(Float a2);

    //0x0060E260
    void _Draw();

    //virtual
    void Draw();

    static inline void *g_vtbl[]{
        func_address(&_Init),
        func_address(&_Update),
        func_address(&_Draw),
    };
};

extern void fe_track_and_field_patch();
