#pragma once

#include "mstring.h"
#include "ngl.h"

struct FEText;
struct PanelAnimFile;
struct PanelFile;
struct PanelQuad;
struct IDirect3DBaseTexture9;

struct FEText;
struct PanelAnimFile;
struct PanelFile;
struct PanelQuad;

struct fe_mission_text {
    std::intptr_t m_vtbl;
    union {
        PanelFile *field_4;
        PanelFile *panel;
    };
    union {
        PanelQuad *field_8;
        PanelQuad *back_1;
    };
    union {
        PanelQuad *field_C;
        PanelQuad *back_2;
    };
    union {
        PanelQuad *field_10;
        PanelQuad *detail;
    };
    union {
        PanelQuad *field_14;
        PanelQuad *back_panel;
    };
    IDirect3DBaseTexture9 *field_18;
    nglTexture field_1C;
    union {
        PanelQuad *field_9C;
        PanelQuad *gradient_text;
    };
    union {
        PanelQuad *field_A0;
        PanelQuad *gradient_hints;
    };
    union {
        PanelQuad *field_A4;
        PanelQuad *gradient_reward;
    };
    union {
        FEText *field_A8;
        FEText *text;
    };
    union {
        PanelAnimFile *field_AC;
        PanelAnimFile *anim;
    };
    int field_B0;
    union {
        bool field_B4;
        bool shown;
    };
    union {
        bool field_B5;
        bool positioned;
    };
    bool field_B6;

    //0x0060D0F0
    fe_mission_text();

    //0x00643A80
    void Init();

    void draw_v10();

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

#ifdef OPENUSM_XBPACK_V10
extern void fe_mission_text_v10_patch();
#endif
