#pragma once

#include <cstdint>

struct nglTexture;

enum nglSortType {
    NGLSORT_OPAQUE = 0,
    NGLSORT_TRANSLUCENT = 1,
};

struct nglSortInfo {
    nglSortType Type;
    union {
        float Dist;
        uint32_t u;
        nglTexture *Tex;
    };

    nglSortInfo() = default;
};
