#pragma once


#include "float.hpp"
#include "ngl_math.h"
#include "vector2d.h"
#include "vector3d.h"

#include <cstdint>

struct nglMeshSection;
struct nglMorphSetSection;

struct nglVertexDef {
    struct IteratorBase {
        std::intptr_t m_vtbl;

        IteratorBase();
    };

    std::intptr_t m_vtbl;
    nglMeshSection *field_4;

    /* virtual */ void Destroy();
};

struct nglVertexDef_Debug_Base {
    struct Iterator : nglVertexDef::IteratorBase {
        int field_4;
        int field_8;
    };
};

struct nglVertexDef_PCUV_Base {
    struct Iterator : nglVertexDef::IteratorBase {
        nglVertexDef *field_4;
        int field_8;

        Iterator();

        void operator++()
        {
            ++this->field_8;
        }
    };
};

struct nglVertexDef_FrontEnd_Base {};

struct nglMaterialBase;

template <typename T>
struct nglVertexDef_MultipassMesh : nglVertexDef {
    struct Iterator : T::Iterator {
        Iterator();

        Iterator(const Iterator &iter);

        Iterator &operator=(const Iterator &) = default;

        Iterator *Clone()
        {
            auto *mem = operator new(0xCu);

            auto *result = new (mem) Iterator{*this};

            return result;
        }

        void BeginStrip(uint32_t);

        void Write(vector3d a2, int color, vector2d);
    };

    Iterator CreateIterator();

    //virtual
    void _Rebase(int);

    //virtual
    void _GenericEdit();

    //virtual
    void _CopyBatches(int);

    //virtual
    void _Destroy();

    //virtual
    void _ApplyMorph(nglMorphSetSection *a2, uint32_t a3, Float a4);
};

struct nglVertexDef_MultipassMesh_Base : nglVertexDef {
    static void AddMeshSection(nglVertexDef *a1, nglMaterialBase *a2, int a3, int a4, int a5, const void *a6,
                               unsigned int a7, int a8, bool a9);
};

extern nglVertexDef::IteratorBase *__fastcall nglVertexDef__GetIterator(void *self, int,
                                                                        nglVertexDef::IteratorBase *a1);


//0x00775AE0
extern void nglCreateMesh(uint32_t Flags, uint32_t num_sections, uint32_t num_bones, math::MatClass<4, 3> *a4);

extern void ngl_vertexdef_patch();
