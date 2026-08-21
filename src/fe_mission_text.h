#pragma once

#include "mstring.h"
#include "ngl.h"

struct FEText;
struct PanelAnimFile;
struct PanelFile;
struct PanelQuad;
struct IDirect3DBaseTexture9;

struct fe_mission_text {
    std::intptr_t m_vtbl;
    PanelFile *field_4;
    PanelQuad *field_8;
    PanelQuad *field_C;
    PanelQuad *field_10;
    PanelQuad *field_14;
    IDirect3DBaseTexture9 *field_18;
    nglTexture field_1C;
    PanelQuad *field_9C;
    PanelQuad *field_A0;
    PanelQuad *field_A4;
    FEText *field_A8;
    PanelAnimFile *field_AC;
    int field_B0;
    bool field_B4;
    bool field_B5;
    bool field_B6;

    //0x0060D0F0
    fe_mission_text();

    //0x00643A80
    void Init();

    //0x0060D560
    void set_flavor(int a2);

    struct string {
        int field_0[2];
        char *data;
        int field_C;
    };

    //0x0060D440
    void set_text(string a2);

    //0x0061AA00
    void SetShown(bool a2);

    static inline void *g_vtbl[]{nullptr};
};

extern void fe_mission_text_patch();
