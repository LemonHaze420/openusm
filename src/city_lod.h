#pragma once

#include "variable.h"

struct nglTexture;
struct nglMesh;
struct generic_mash_header;
struct generic_mash_data_ptrs;

struct strip_lod {
    int field_0;
    int field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;

    //0x0052F380
    void un_mash_start(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3, void *a4);
};

struct city_lod {
    strip_lod *field_0;
    bool field_4;

    //0x00550420
    city_lod(const char *a1);

    //0x00540380
    void render();

    static inline Var<nglMesh *> map_mesh{0x0095C188};
    static inline Var<nglMesh *> sides_mesh{0x0095C16C};
    static inline Var<nglMesh *> top_mesh{0x0095C748};
    static inline Var<nglMesh *> ground_mesh{0x0095C714};
    static inline Var<nglMesh *> box_mesh{0x0095C1d8};
    static inline Var<nglTexture *> top_texture{0x0095C710};
};

extern void city_lod_patch();
