#pragma once

#include "nglrendernode.h"

#include <tl_system.h>
#include <variable.h>

#include <cstdint>

struct color;
struct nglMeshNode;
struct nglMeshSection;
struct nglMaterialBase;
struct tlFixedString;

template <typename>
struct nglParamSet;

struct nglShaderParamSet_Pool;

struct nglShader : tlInitList {
    int field_8;

    nglShader();

    //virtual
    void _Register();

    /* virtual */ tlFixedString GetName();

    //virtual
    void AddNode(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    /* virtual */ void BindMaterial(nglMaterialBase *mat);

    /* virtual */ void ReleaseMaterial(nglMaterialBase *mat);

    /* virtual */ void RebaseMaterial(nglMaterialBase *mat, unsigned int a2);

    bool _CheckMaterialVersion(nglMaterialBase *mat);

    /* virtual */ bool CheckMaterialVersion(nglMaterialBase *mat);

    bool _CheckVertexDefVersion(nglMeshSection *Section);

    /* virtual */ bool CheckVertexDefVersion(nglMeshSection *Section);

    void _BindSection(nglMeshSection *);

    /* virtual */ void BindSection(nglMeshSection *Section);

    /* virtual */ bool IsSwitchable();

    static int &NextID;
};

struct nglShaderNode : nglRenderNode {
    nglMeshNode *m_meshNode;
    nglMeshSection *m_meshSection;

    nglShaderNode(nglMeshNode *a2, nglMeshSection *a3);

    void sub_413AF0();

    void Render();
};

extern void sub_417C10(nglShaderNode *a1);

extern color *sub_413F80(color *a1, nglMaterialBase *a2, nglParamSet<nglShaderParamSet_Pool> *a3, uint32_t a4);

extern void nglShader_patch();
