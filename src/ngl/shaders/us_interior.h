#pragma once

struct nglTexture;
struct tlFixedString;

struct USInteriorMaterial {
    char field_0[0x1C];
    tlFixedString *field_1C;
    nglTexture *field_20;
};
