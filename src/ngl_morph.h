#pragma once

#include "float.hpp"

#include <cstdint>

struct nglMesh;
struct nglMeshSection;

struct nglMorph {
    int m_vtbl;

    //virtual
    bool IsMeshMorph() const;

    //virtual
    void Apply(nglMeshSection *a2, int a3, Float a4, uint32_t a5);

    //virtual
    int GetComponentMask(uint32_t a2) const;
};

struct nglMeshMorph {
    int m_vtbl;
    nglMesh *field_4;
};

struct nglMorphFrame : nglMorph {
    void *field_4;

    nglMorphFrame() = default;
    explicit nglMorphFrame(void *frame);
};

struct nglMorphEntry {
    float field_0;
    nglMorph *Morph;
};

extern void nglBlendMorphs(nglMesh *a1, uint32_t a2, nglMorphEntry *Morphs);
