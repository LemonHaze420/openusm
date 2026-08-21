#include "ngl_vertexdef.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "ngl_dx_vertexdef.h"
#include "ngl_mesh.h"
#include "osassert.h"
#include "trace.h"
#include "vector4d.h"
#include "vtbl.h"

using def_t = nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>;
using iterator_type = typename nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator;

VALIDATE_SIZE(def_t, 0x8);
VALIDATE_SIZE(iterator_type, 0xC);

nglVertexDef::IteratorBase::IteratorBase()
{
    this->m_vtbl = 0x0086F864;
}


void nglVertexDef::Destroy()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
    func(this);
}

nglVertexDef_PCUV_Base::Iterator::Iterator()
{
    this->m_vtbl = 0x00870AF0;
    this->field_8 = 0;
}

template <>
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Iterator()
{
    this->m_vtbl = 0x0087188C;
}

template <>
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Iterator(
    const nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator &iter)
{
    THISCALL(0x004121E0, this, &iter);
}

template <>
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::CreateIterator()
{
    TRACE("nglVertexDef::CreateIterator");

    Iterator result;
    THISCALL(0x00413C60, this, &result);

    return result;
}

template <>
void nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::_ApplyMorph(nglMorphSetSection *a2, uint32_t a3, Float a4)
{
    TRACE("nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::ApplyMorph");

    THISCALL(0x004083F0, this, a2, a3, a4);
}

template <>
void nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::BeginStrip(uint32_t nVerts)
{
    ::BeginStrip(nVerts, 0x18);
}

template <>
void nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Write(vector3d a2, int color, vector2d a4)
{
    TRACE("nglVertexDef::Iterator::Write");

    if constexpr (1) {
        struct {
            vector3d field_0;
            float field_C[2];
            uint32_t field_14;
        } *v5 = CAST(v5, int(this->field_4->field_4->field_3C.getVertexData()) + this->field_4->field_4->field_4C) +
                this->field_8;
        v5->field_0 = a2;
        v5->field_C[0] = a4[0];
        v5->field_C[1] = a4[1];
        v5->field_14 = color;
    } else {
        THISCALL(0x00501EE0, this, a2, color, a4);
    }
}


void nglVertexDef_MultipassMesh_Base::AddMeshSection(nglVertexDef *vertexDef, nglMaterialBase *a2, int numVertices,
                                                     int a4, int a5, const void *a6, unsigned int stride,
                                                     int primitiveType, bool a9)
{
    TRACE("nglVertexDef_MultipassMesh_Base::AddMeshSection");

    if constexpr (0) {
        if (nglScratchSectionIdx() >= nglScratch()->NSections) {
            error("Added too many sections to a scratch mesh.\n");
        }

        auto *Section = nglScratch()->Sections[nglScratchSectionIdx()].Section;
        vertexDef->field_4 = Section;
        Section->VertexDef = vertexDef;
        Section->Material = a2;
        Section->m_primitiveType = static_cast<decltype(Section->m_primitiveType)>(primitiveType);
        Section->m_stride = stride;
        if (a9) {
            Section->StartIndex = nglScratchBuffer().field_30;
            Section->m_indexBuffer = nglScratchBuffer().field_48;
        } else {
            Section->NIndices = 0;
            Section->NVertices = numVertices;
        }

        if ((nglScratch()->Flags & NGLMESH_TEMP) != 0) {
            Section->Flags = NGLMESH_TEMP;
            auto v10 = nglScratchBuffer().field_28;
            if (nglScratchBuffer().field_28 % stride) {
                v10 = stride - nglScratchBuffer().field_28 % stride + nglScratchBuffer().field_28;
                nglScratchBuffer().field_28 = v10;
            }

            nglScratchBuffer().field_2C = v10;
            Section->field_4C = nglScratchBuffer().GetVertexOffset();
            Section->field_3C = nglScratchBuffer().field_4C;
            nglScratchBuffer().field_28 += stride * numVertices;

            assert(nglScratchBuffer().GetVertexOffset() < nglPhysListWorkSize() &&
                   "Temp scratch mesh VB overflow (increase NGLBUF_PHYSICAL_LIST_WORK)");
        } else {
            nglVertexBuffer::createIndexOrVertexBuffer(
                &Section->field_3C, ResourceType::VertexBuffer, stride * numVertices, 0, 0, D3DPOOL_MANAGED);
        }

        if (a5) {
            auto *v11 = static_cast<uint16_t *>(nglMeshAllocFn()(2 * a5, 16, 0));
            Section->BonesIdx = v11;
            std::memcpy(Section->BonesIdx, a6, 2 * a5);
            Section->NBones = a5;
        }

        static Var<nglMeshSection *> dword_972A10{0x00972A10};
        dword_972A10() = Section;
        ++nglScratchSectionIdx();
    } else {
        CDECL_CALL(0x00771D10, vertexDef, a2, numVertices, a4, a5, a6, stride, primitiveType, a9);
    }
}

void sub_733030(nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator *iter, const vector4d &a2,
                const vector4d &a3, const vector4d &a4, const vector4d &a5, int a6, int p_color)
{
    TRACE("sub_733030");

    ::BeginStrip(4u, 24u);
    auto v7 = iter->field_4->field_4;
    auto v8 = a2[3];
    auto v10 = v7->field_4C + 0x18 * iter->field_8;
    struct VertexData {
        char field_0[4];
        char field_4[4];
        char field_8[4];
        char field_C[4];
        char field_10[4];
        char field_14[4];
    };

    VALIDATE_SIZE(VertexData, 0x18);

    auto *v11 = bit_cast<VertexData *>(v7->field_3C.getVertexData());

    *(float *)&v11->field_0[v10] = a2[0];
    *(float *)&v11->field_4[v10] = a2[1];
    *(float *)&v11->field_C[v10] = a2[2];
    *(DWORD *)&v11->field_8[v10] = a6;
    *(float *)&v11->field_10[v10] = v8;
    *(DWORD *)&v11->field_14[v10] = p_color;
    ++iter->field_8;
    auto v14 = iter->field_8;
    v14 *= 3;

    auto *v13 = iter->field_4;
    VertexData *v18 = CAST(v18, v13->field_4->field_3C.getVertexData() + v13->field_4->field_4C);

    *(float *)&v18->field_0[8 * v14] = a3[0];
    *(float *)&v18->field_4[8 * v14] = a3[1];
    *(float *)&v18->field_C[8 * v14] = a3[2];
    *(DWORD *)&v18->field_8[8 * v14] = a6;
    *(float *)&v18->field_10[8 * v14] = a3[3];
    *(DWORD *)&v18->field_14[8 * v14] = p_color;

    ++iter->field_8;

    auto *v19 = iter->field_4;
    int v20 = iter->field_8;
    VertexData *v24 = CAST(v24, v19->field_4->field_3C.getVertexData() + 0x18 * v20 + v19->field_4->field_4C);

    *(float *)v24->field_0 = a4[0];
    *(float *)v24->field_4 = a4[1];
    *(float *)v24->field_C = a4[2];
    *(DWORD *)v24->field_8 = a6;
    *(float *)v24->field_10 = a4[3];
    *(DWORD *)v24->field_14 = p_color;

    ++iter->field_8;

    auto v25 = iter->field_8;
    VertexData *v29 =
        CAST(v29, iter->field_4->field_4->field_3C.getVertexData() + 0x18 * v25 + iter->field_4->field_4->field_4C);

    *(float *)v29->field_0 = a5[0];
    *(float *)v29->field_4 = a5[1];
    *(float *)v29->field_C = a5[2];
    *(DWORD *)v29->field_8 = a6;
    *(float *)v29->field_10 = a5[3];
    *(DWORD *)v29->field_14 = p_color;
    ++iter->field_8;
}


void nglCreateMesh(uint32_t Flags, uint32_t num_sections, uint32_t num_bones, math::MatClass<4, 3> *a4)
{
    TRACE("nglCreateMesh");

    assert(((Flags & NGLMESH_TEMP) || (Flags & NGLMESH_STATIC)) &&
           "Either NGLMESH_TEMP or NGLMESH_STATIC mesh flag mush be specified to nglCreateMesh "
           "!\n");

    if constexpr (1) {
        nglMeshAllocFn() = ((Flags & 0x40000) != 0 ? nglMeshScratchAlloc : nglMeshMemAlloc);
        ;
        auto *Mesh = static_cast<nglMesh *>(nglMeshAllocFn()(sizeof(nglMesh), 16, 0));

#if 1
        Mesh = new (Mesh) nglMesh{};
#else
        std::memset(Mesh, 0, sizeof(nglMesh));
#endif

        Mesh->Flags = Flags | NGLMESH_SCRATCH_MESH;
        Mesh->NSections = num_sections;
        Mesh->Sections =
            static_cast<decltype(Mesh->Sections)>(nglMeshAllocFn()(sizeof(*Mesh->Sections) * num_sections, 16, 0));
        if (Mesh->Sections != nullptr) {
            for (auto v5 = 0u; v5 < num_sections; ++v5) {
                Mesh->Sections[v5].Section =
                    static_cast<nglMeshSection *>(nglMeshAllocFn()(sizeof(nglMeshSection), 16, 0));
                Mesh->Sections[v5].Section = {};
                Mesh->Sections[v5].field_0 = 1;
            }

            if (num_bones != 0) {
                Mesh->NBones = num_bones;
                Mesh->Bones =
                    static_cast<math::MatClass<4, 3> *>(nglMeshAllocFn()(sizeof(*Mesh->Bones) * num_bones, 16, 0));
                Mesh->Bones = a4;
            }

            nglScratch() = Mesh;
            nglScratchSectionIdx() = 0;

            nglMeshSetSphere(math::Float4_0001, 1.0e32);
            if ((Flags & NGLMESH_STATIC) != 0) {
                [](auto &buffer) -> void {
                    buffer.field_34 = buffer.m_numVertices;
                    buffer.field_38 = buffer.field_28;
                    buffer.field_3C = buffer.field_2C;
                    buffer.field_40 = buffer.field_30;
                }(nglScratchBuffer());
            }
        } else {
            nglScratch() = nullptr;
            nglScratchSectionIdx() = 0;
        }
    } else {
        CDECL_CALL(0x00775AE0, Flags, num_sections, num_bones, a4);
    }
}

void ngl_vertexdef_patch()
{
    {
        REDIRECT(0x00507962, nglVertexDef_MultipassMesh_Base::AddMeshSection);
    }

    SET_JUMP(0x00733030, sub_733030);

    {
        SET_JUMP(0x00775AE0, nglCreateMesh);
    }

    {
        FUNC_ADDRESS(address, &nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Write);
        SET_JUMP(0x00501EE0, address);
    }

    {
        auto func = &nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::_ApplyMorph;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871A70, address);
    }
}
