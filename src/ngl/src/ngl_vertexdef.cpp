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
#include "tl_system.h"
#include "tl_instance_bank.h"
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <functional>

extern "C" int __cdecl _purecall();

template <>
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Iterator(
    const nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator &);
template <>
void nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::BeginStrip(uint32_t);
template <>
void nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Write(vector3d, int, vector2d);

using def_t = nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>;
using iterator_type = typename nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator;

VALIDATE_SIZE(def_t, 0x8);
VALIDATE_SIZE(iterator_type, 0xC);

namespace {
void __fastcall destroy_pcuv_iterator(iterator_type *self, void *, unsigned char flags)
{
    if ((flags & 1) != 0) {
        operator delete(self);
    }
}

iterator_type *__fastcall clone_pcuv_iterator(iterator_type *self, void *)
{
    return self->Clone();
}

bool __fastcall test_pcuv_iterator(iterator_type *self, void *)
{
    return static_cast<uint32_t>(self->field_8) < static_cast<uint32_t>(self->field_4->field_4->NVertices);
}

std::intptr_t pcuv_iterator_table()
{
    static void *table[]{reinterpret_cast<void *>(destroy_pcuv_iterator),
                         reinterpret_cast<void *>(clone_pcuv_iterator),
                         reinterpret_cast<void *>(test_pcuv_iterator)};
    return reinterpret_cast<std::intptr_t>(table);
}
}  // namespace

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
    this->m_vtbl = pcuv_iterator_table();
}

template <>
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator::Iterator(
    const nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator &iter)
{
    this->m_vtbl = iter.m_vtbl;
    this->field_4 = iter.field_4;
    this->field_8 = iter.field_8;
}

template <>
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::Iterator
nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::CreateIterator()
{
    auto *section = this->field_4;
    if ((section->Flags & NGLMESH_TEMP) == 0) {
        void *data{};
        IDirect3DVertexBuffer9_Lock(section->field_3C.getVertexBuffer(), 0, 0, &data, 0);
        section->field_3C.setVertexData(static_cast<char *>(data));
    }
    Iterator result;
    result.field_4 = this;
    result.field_8 = 0;
    return result;
}

template <>
void nglVertexDef_MultipassMesh<nglVertexDef_PCUV_Base>::_ApplyMorph(nglMorphSetSection *a2, uint32_t a3, Float a4)
{
    auto *section = field_4;
    auto *vertices = section->field_3C.getVertexData() + section->field_4C;
    const auto apply_component =
        [&](uint32_t mask, uint32_t stream_slot, uint32_t offset, uint32_t dimensions, uint32_t source_stride) {
            if (a2 != nullptr && (a3 & mask) == 0) {
                return;
            }
            if (a2 == nullptr) {
                if (std::not_equal_to<float>{}(a4, 1.0f)) {
                    for (int i = 0; i < section->NVertices; ++i) {
                        auto *out = reinterpret_cast<float *>(vertices + 24 * i + offset);
                        for (uint32_t c = 0; c < dimensions; ++c) {
                            out[c] *= a4;
                        }
                    }
                }
                return;
            }
            auto *source = reinterpret_cast<const char *const *>(a2)[stream_slot];
            uint32_t vertex = 0;
            uint16_t skip;
            do {
                uint16_t count;
                std::memcpy(&count, source, 2);
                std::memcpy(&skip, source + 2, 2);
                source += 4;
                for (uint16_t i = 0; i < count; ++i, ++vertex, source += source_stride) {
                    auto *out = reinterpret_cast<float *>(vertices + 24 * vertex + offset);
                    const auto *delta = reinterpret_cast<const float *>(source);
                    for (uint32_t c = 0; c < dimensions; ++c) {
                        out[c] += a4 * delta[c];
                    }
                }
                if ((reinterpret_cast<std::uintptr_t>(source) & 1) != 0) {
                    ++source;
                }
                vertex += skip;
            } while (skip != 0);
        };
    apply_component(1, 2, 0, 3, 12);
    if (a2 == nullptr || (a3 & 4) != 0) {
        const auto write_color = [&](uint32_t vertex, const float *delta) {
            auto *packed = reinterpret_cast<uint32_t *>(vertices + 24 * vertex + 20);
            uint32_t result{};
            for (uint32_t channel = 0; channel < 4; ++channel) {
                const uint32_t shift = channel == 0 ? 16 : channel == 2 ? 0 : 8 * channel;
                float value = static_cast<float>((*packed >> shift) & 255) * 0.0039215689f;
                if (delta != nullptr) {
                    value = std::min(1.0f, std::max(0.0f, value + a4 * delta[channel]));
                } else {
                    value *= a4;
                }
                result |= (static_cast<uint32_t>(value * 255.0f) & 255) << shift;
            }
            *packed = result;
        };
        if (a2 == nullptr) {
            if (std::not_equal_to<float>{}(a4, 1.0f)) {
                for (int i = 0; i < section->NVertices; ++i) {
                    write_color(i, nullptr);
                }
            }
        } else {
            auto *source = reinterpret_cast<const char *const *>(a2)[4];
            uint32_t vertex{};
            uint16_t skip;
            do {
                uint16_t count;
                std::memcpy(&count, source, 2);
                std::memcpy(&skip, source + 2, 2);
                source += 4;
                for (uint16_t i = 0; i < count; ++i, ++vertex, source += 16) {
                    write_color(vertex, reinterpret_cast<const float *>(source));
                }
                if ((reinterpret_cast<std::uintptr_t>(source) & 1) != 0) {
                    ++source;
                }
                vertex += skip;
            } while (skip != 0);
        }
    }
    apply_component(0x40, 8, 12, 2, 12);
    if ((section->Flags & NGLMESH_TEMP) == 0) {
        IDirect3DVertexBuffer9_Unlock(section->field_3C.getVertexBuffer());
    }
}

namespace {
void __fastcall rebase_pcuv(def_t *self, void *, int offset)
{
    self->field_4 = reinterpret_cast<nglMeshSection *>(reinterpret_cast<char *>(self->field_4) + offset);
}

iterator_type **__fastcall edit_pcuv(def_t *self, void *, iterator_type **out)
{
    iterator_type iter;
    iter.field_4 = self;
    iter.field_8 = 0;
    *out = iter.Clone();
    return out;
}

def_t *__fastcall copy_pcuv(def_t *, void *, nglMeshSection *section);

void __fastcall destroy_pcuv(def_t *self, void *)
{
    tlMemFree(self);
}
}  // namespace

static std::intptr_t pcuv_vertex_table()
{
    static void *table[]{reinterpret_cast<void *>(rebase_pcuv),
                         reinterpret_cast<void *>(edit_pcuv),
                         reinterpret_cast<void *>(copy_pcuv),
                         reinterpret_cast<void *>(destroy_pcuv),
                         func_address(&def_t::_ApplyMorph)};
    return reinterpret_cast<std::intptr_t>(table);
}

static def_t *process_pcuv(def_t *def)
{
    if (def != nullptr) {
        def->m_vtbl = pcuv_vertex_table();
    }
    return def;
}

void nglRegisterPCUVVertexDef()
{
    nglVertexDefBank.Insert(tlFixedString{"PCUV"}, reinterpret_cast<void *>(process_pcuv));
}

def_t *nglCreatePCUVVertexDef()
{
    auto *def = static_cast<def_t *>(nglMeshAllocFn()(sizeof(def_t), 4, 0));
    if (def != nullptr) {
        def->m_vtbl = pcuv_vertex_table();
    }
    return def;
}

namespace {
def_t *__fastcall copy_pcuv(def_t *self, void *, nglMeshSection *section)
{
    auto *def = static_cast<def_t *>(tlMemAlloc(sizeof(def_t), 8, 0));
    if (def != nullptr) {
        def->m_vtbl = self->m_vtbl;
    }
    def->field_4 = section;
    return def;
}
}  // namespace

void nglAddPCUVTriangle(nglMaterialBase *material, const vector3d (&positions)[3], const vector2d (&uv)[3],
                        const uint32_t (&colors)[3])
{
    auto *def = nglCreatePCUVVertexDef();
    nglVertexDef_MultipassMesh_Base::AddMeshSection(def, material, 3, 1, 0, nullptr, 24, D3DPT_TRIANGLESTRIP, true);
    auto iter = def->CreateIterator();
    iter.BeginStrip(3);
    for (uint32_t i = 0; i < 3; ++i) {
        iter.Write(positions[i], colors[i], uv[i]);
        ++iter;
    }
    if ((def->field_4->Flags & NGLMESH_TEMP) == 0) {
        IDirect3DVertexBuffer9_Unlock(def->field_4->field_3C.getVertexBuffer());
    }
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

    if constexpr (STANDALONE_SYSTEM) {
        if (static_cast<uint32_t>(nglScratchSectionIdx()) >= nglScratch()->NSections) {
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
                new (Mesh->Sections[v5].Section) nglMeshSection{};
                Mesh->Sections[v5].field_0 = 1;
            }

            if (num_bones != 0) {
                Mesh->NBones = num_bones;
                Mesh->Bones =
                    static_cast<math::MatClass<4, 3> *>(nglMeshAllocFn()(sizeof(*Mesh->Bones) * num_bones, 16, 0));
                for (uint32_t bone = 0; bone < num_bones; ++bone) {
                    new (Mesh->Bones + bone) math::MatClass<4, 3>{a4[bone]};
                }
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

namespace {


struct PersonVertexIterator {
    std::intptr_t m_vtbl;
    nglVertexDef *definition;
    uint32_t vertex;
};

VALIDATE_SIZE(nglVertexDef, 8);
VALIDATE_SIZE(PersonVertexIterator, 12);

std::intptr_t person_iterator_base_table();

PersonVertexIterator *__fastcall destroy_person_iterator(PersonVertexIterator *self, void *, unsigned char flags)
{
    self->m_vtbl = person_iterator_base_table();
    if ((flags & 1) != 0) {
        operator delete(self);
    }
    return self;
}


void __fastcall person_iterator_empty_one(PersonVertexIterator *, void *, uint32_t) {}
void __fastcall person_iterator_empty_three(PersonVertexIterator *, void *, uint32_t, uint32_t, uint32_t) {}

std::intptr_t person_iterator_base_table()
{
    static void *table[]{reinterpret_cast<void *>(destroy_person_iterator),
                         reinterpret_cast<void *>(_purecall),
                         reinterpret_cast<void *>(_purecall),
                         reinterpret_cast<void *>(_purecall),
                         reinterpret_cast<void *>(person_iterator_empty_three),
                         reinterpret_cast<void *>(person_iterator_empty_three)};
    return reinterpret_cast<std::intptr_t>(table);
}

PersonVertexIterator *__fastcall clone_person_iterator(PersonVertexIterator *self, void *)
{
    return new PersonVertexIterator{*self};
}

int __fastcall test_person_iterator(PersonVertexIterator *self, void *)
{
    return self->vertex < static_cast<uint32_t>(self->definition->field_4->NVertices);
}

template <uint32_t Stride>
std::intptr_t person_iterator_table()
{
    static void *table[]{reinterpret_cast<void *>(destroy_person_iterator),
                         reinterpret_cast<void *>(clone_person_iterator),
                         reinterpret_cast<void *>(test_person_iterator),
                         reinterpret_cast<void *>(person_iterator_empty_one),
                         reinterpret_cast<void *>(person_iterator_empty_three),
                         reinterpret_cast<void *>(person_iterator_empty_three),
                         reinterpret_cast<void *>(person_iterator_empty_one)};
    return reinterpret_cast<std::intptr_t>(table);
}

int __fastcall rebase_person_vertexdef(nglVertexDef *self, void *, int offset)
{
    self->field_4 = reinterpret_cast<nglMeshSection *>(reinterpret_cast<char *>(self->field_4) + offset);
    return offset;
}

template <uint32_t Stride>
PersonVertexIterator **__fastcall edit_person_vertexdef(nglVertexDef *self, void *, PersonVertexIterator **out)
{
    PersonVertexIterator iterator{person_iterator_table<Stride>(), self, 0};
    *out = clone_person_iterator(&iterator, nullptr);
    return out;
}

void __fastcall destroy_person_vertexdef(nglVertexDef *self, void *)
{
    tlMemFree(self);
}

template <uint32_t Stride>
std::intptr_t person_vertex_table();

template <uint32_t Stride>
nglVertexDef *__fastcall copy_person_vertexdef(nglVertexDef *, void *, nglMeshSection *section)
{
    auto *definition = static_cast<nglVertexDef *>(tlMemAlloc(sizeof(nglVertexDef), 8, 0));
    if (definition != nullptr) {
        definition->m_vtbl = person_vertex_table<Stride>();
    }
    definition->field_4 = section;
    return definition;
}

template <uint32_t Stride>
void __fastcall morph_person_vertexdef(nglVertexDef *self, void *, nglMorphSetSection *morph, uint32_t mask,
                                       float weight)
{
    auto *section = self->field_4;
    if ((section->Flags & NGLMESH_TEMP) == 0) {
        void *data{};
        IDirect3DVertexBuffer9_Lock(section->field_3C.getVertexBuffer(), 0, 0, &data, 0);
        section->field_3C.setVertexData(static_cast<char *>(data));
    }
    auto *vertices = section->field_3C.getVertexData() + section->field_4C;
    if (morph == nullptr) {
        if (std::not_equal_to<float>{}(weight, 1.0f)) {
            for (uint32_t vertex = 0; vertex < static_cast<uint32_t>(section->NVertices); ++vertex) {
                auto *position = reinterpret_cast<float *>(vertices + Stride * vertex);
                position[0] *= weight;
                position[1] *= weight;
                position[2] *= weight;
            }
        }
    } else if ((mask & 1) != 0) {
        auto *source = reinterpret_cast<const char *const *>(morph)[2];
        uint32_t vertex{};
        uint16_t skip;
        do {
            uint16_t count;
            std::memcpy(&count, source, sizeof(count));
            std::memcpy(&skip, source + 2, sizeof(skip));
            source += 4;
            for (uint16_t i = 0; i < count; ++i, ++vertex, source += 12) {
                auto *position = reinterpret_cast<float *>(vertices + Stride * vertex);
                const auto *delta = reinterpret_cast<const float *>(source);
                position[0] += weight * delta[0];
                position[1] += weight * delta[1];
                position[2] += weight * delta[2];
            }
            if ((reinterpret_cast<std::uintptr_t>(source) & 1) != 0) {
                ++source;
            }
            vertex += skip;
        } while (skip != 0);
    }
    if ((section->Flags & NGLMESH_TEMP) == 0) {
        IDirect3DVertexBuffer9_Unlock(section->field_3C.getVertexBuffer());
    }
}

template <uint32_t Stride>
std::intptr_t person_vertex_table()
{
    static void *table[]{reinterpret_cast<void *>(rebase_person_vertexdef),
                         reinterpret_cast<void *>(edit_person_vertexdef<Stride>),
                         reinterpret_cast<void *>(copy_person_vertexdef<Stride>),
                         reinterpret_cast<void *>(destroy_person_vertexdef),
                         reinterpret_cast<void *>(morph_person_vertexdef<Stride>)};
    return reinterpret_cast<std::intptr_t>(table);
}

template <uint32_t Stride>
nglVertexDef *process_person_vertexdef(nglVertexDef *definition)
{
    if (definition != nullptr) {
        definition->m_vtbl = person_vertex_table<Stride>();
    }
    return definition;
}
}  // namespace

void nglRegisterPersonVertexDefs()
{
    nglVertexDefBank.Insert(tlFixedString{"USPersonMorphable"}, reinterpret_cast<void *>(process_person_vertexdef<64>));
    nglVertexDefBank.Insert(tlFixedString{"USPersonMorphable_NickFuryEye"},
                            reinterpret_cast<void *>(process_person_vertexdef<64>));
    nglVertexDefBank.Insert(tlFixedString{"USMShinyMorphable"}, reinterpret_cast<void *>(process_person_vertexdef<60>));
    nglVertexDefBank.Insert(tlFixedString{"USMSimpleMorphable"},
                            reinterpret_cast<void *>(process_person_vertexdef<24>));
    nglVertexDefBank.Insert(tlFixedString{"us_grunge_morphable"},
                            reinterpret_cast<void *>(process_person_vertexdef<32>));
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
