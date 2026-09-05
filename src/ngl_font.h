#pragma once

#include "fixedstring.h"
#include "float.hpp"
#include "ngl.h"
#include "variable.h"

struct nglTexture;

struct nglGlyphInfo {
    uint32_t TexOfs[2];
    uint32_t GlyphSize[2];
    int32_t GlyphOrigin[2];
    uint32_t CellWidth;
};

struct nglGlyphSize {
    float field_0;
    float field_4;
    float field_8;
    float field_C;
};

struct nglFont {
    tlFixedString field_0;
    int field_20;
    nglTexture *field_24;
    nglGlyphInfo *GlyphInfo;

    struct {
        int Version;
        int CellHeight;
        int Ascent;
        int FirstGlyph;
        int NumGlyphs;

    } Header;

    int field_40;
    nglBlendModeType m_blend_mode;
    int field_48;

    nglGlyphSize *field_4C;
    char field_50;
    char empty0[3];
    static tlFixedString *get_string(nglFont *font)
    {
        return &font->field_0;
    }


    int GetFontCellWidth(uint8_t a2)
    {
        return this->GetGlyphInfo(a2)->CellWidth;
    }

    nglGlyphInfo *GetGlyphInfo(unsigned char Character);

    void sub_77E2F0(uint8_t a2, float *a3, float *a4, float *a5, float *a6, Float a7, Float a8);
};

extern Var<nglFont *> nglSysFont;

//0x00778F80
extern void nglParseFDF(char *a3, nglFont *a2);
