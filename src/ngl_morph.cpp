#include "ngl_morph.h"

#include "common.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_vertexdef.h"
#include "vtbl.h"

VALIDATE_SIZE(nglMorph, 0x4);

VALIDATE_SIZE(nglMeshMorph, 0x8);

VALIDATE_SIZE(nglMorphFrame, 0x8);

namespace {
bool __fastcall frame_is_mesh_morph(const nglMorphFrame *, void *)
{
    return false;
}

uint32_t __fastcall frame_component_mask(const nglMorphFrame *self, void *, uint32_t section)
{
    const auto *frame = static_cast<decltype(nglMorphSet::Frames)>(self->field_4);
    return frame->field_8[section].field_4;
}

void __fastcall apply_morph_frame(nglMorphFrame *self, void *, nglMeshSection *section,
                                 int section_index, Float weight, uint32_t mask)
{
    if (section->VertexDef != nullptr) {
        auto *frame = static_cast<decltype(nglMorphSet::Frames)>(self->field_4);
        using apply_fn = void(__fastcall *)(nglVertexDef *, void *, nglMorphSetSection *, uint32_t, Float);
        auto apply = reinterpret_cast<apply_fn>(get_vfunc(section->VertexDef->m_vtbl, 0x10));
        apply(section->VertexDef, nullptr,
              reinterpret_cast<nglMorphSetSection *>(frame->field_8 + section_index), mask, weight);
    }
}
}

nglMorphFrame::nglMorphFrame(void *frame)
{
    static void *table[] = {reinterpret_cast<void *>(frame_is_mesh_morph),
                           reinterpret_cast<void *>(apply_morph_frame),
                           reinterpret_cast<void *>(frame_component_mask)};
    m_vtbl = reinterpret_cast<int>(table);
    field_4 = frame;
}

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
