#include "ngl_morph.h"

#include "common.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "trace.h"
#include "vtbl.h"

VALIDATE_SIZE(nglMorph, 0x4);

VALIDATE_SIZE(nglMeshMorph, 0x8);

VALIDATE_SIZE(nglMorphFrame, 0x8);

bool nglMorph::IsMeshMorph() const
{
    bool(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0x0));
    return func(this);
}

void nglMorph::Apply(nglMeshSection *a2, int a3, Float a4, uint32_t a5)
{
    void(__fastcall * func)(void *, void *, nglMeshSection *, int, Float, uint32_t) =
        CAST(func, get_vfunc(m_vtbl, 0x4));
    func(this, nullptr, a2, a3, a4, a5);
}

int nglMorph::GetComponentMask(uint32_t a2) const
{
    uint32_t(__fastcall * func)(const void *, void *, uint32_t) = CAST(func, get_vfunc(m_vtbl, 0x8));
    return func(this, nullptr, a2);
}

void nglBlendMorphs(nglMesh *Mesh, uint32_t a2, nglMorphEntry *Morphs)
{
    TRACE("nglBlendMorphs");

    if (a2 != 0) {
        if (Morphs->Morph->IsMeshMorph()) {
            nglMeshMorph *v1 = CAST(v1, Morphs->Morph);
            if (auto *srcMesh = v1->field_4; srcMesh != Mesh) {
                nglCopyMesh(Mesh, srcMesh);
            }
        }

        auto size = 4 * a2 * Mesh->NSections;
        auto *buf = static_cast<uint32_t *>(nglListAlloc(size, 16));

        for (auto i = 0u; i < Mesh->NSections; ++i) {
            for (auto j = 0u; j < a2; ++j) {
                buf[j + a2 * i] = Morphs[j].Morph->GetComponentMask(i);
            }
        }

        for (auto i = 0u; i < Mesh->NSections; ++i) {
            for (auto j = 0u; j < a2; ++j) {
                assert(Mesh->Sections[i].Section->VertexDef != nullptr && "Mesh section is not morphable.");

                assert((j == 0 || !Morphs[j].Morph->IsMeshMorph()) &&
                       "Only the first blend entry can be a mesh morph.");

                Morphs[j].Morph->Apply(Mesh->Sections[i].Section, i, Morphs[j].field_0, buf[j + a2 * i]);
            }
        }

        nglListWorkPos() -= size;
    }
}
