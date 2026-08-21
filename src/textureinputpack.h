#pragma once

#include "color32.h"

#include <cstdint>

struct Input;

struct TexturePackBase {
    std::intptr_t m_vtbl;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;
    int field_18;

    struct {
        std::intptr_t m_vtbl;
    } *field_1C;
    int field_20[19];
};

struct IDirect3DBaseTexture9;

struct TextureInputPack : TexturePackBase {
    int field_6C;
    int field_70;
    int field_74;
    IDirect3DBaseTexture9 *field_78;
    IDirect3DBaseTexture9 *field_7C;
    IDirect3DBaseTexture9 *field_80;
    IDirect3DBaseTexture9 *field_84;
    int field_88;
    int field_8C;
    int field_90;
    int field_94;
    int field_98;
    int field_9C;
    char field_A0[4];

    int field_A4[199];
    float field_3C0;
    float field_3C4;
    float field_3C8;
    float field_3CC;
    int field_3D0;
    int field_3D4;
    int field_3D8;
    float field_3DC;
    int field_3E0;
    int field_3E4;
    int field_3E8;
    color32 field_3EC;
    color32 field_3F0;
    color32 field_3F4;
    color32 field_3F8;
    color32 field_3FC;
    int field_400;
    int field_404;
    int field_408;
    int field_40C;
    int field_410;
    int field_414;
    Input *field_418;
    int field_41C[100];
    char *field_5AC;

    TextureInputPack();

    /* virtual */ void sub_5871D0();
};

extern void TextureInputPack_patch();
