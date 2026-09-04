#pragma once

#include "fixedstring.h"
#include "nglshader.h"
#include "ngl.h"
#include "vector4d.h"

#include <us_variant.h>

struct nglMaterialBase;
struct nglTexture;
struct nglMeshFile;

namespace USPersonShaderSpace {

struct USPersonMaterial {
    tlFixedString *Name;
    nglShader *m_shader;
    nglMeshFile *File;
    nglMaterialBase *NextMaterial;
    int field_10;
    int field_14;
    tlFixedString *field_18;
    nglTexture *field_1C;
    tlFixedString *field_20;
    nglTexture *field_24;
    vector4d field_28;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int m_outlineFeature;
    nglBlendModeType m_blend_mode;
    int field_50;
};

struct USPersonMaterialSolid {
    tlFixedString *Name;
    nglShader *m_shader;
    nglMeshFile *File;
    nglMaterialBase *NextMaterial;
    int field_10;
    int field_14;
    tlFixedString *field_18;
    nglTexture *field_1C;
    tlFixedString *field_20;
    nglTexture *field_24;
    float field_28[4];
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int m_outlineFeature;
    nglBlendModeType m_blend_mode;
    int field_50;
};

struct ParamStruct {
    float field_0[4];
    float field_10[4];
    float field_20[4];
    float field_30;
    float field_34;
    bool field_38;
    int field_3C;
    bool field_40;
    bool field_41;
    int field_44;
    bool field_48;
    bool disableZDepth;

    void *operator new(size_t size);

    static inline constexpr auto OutlineThickness = 0.003f;
};

extern Var<ParamStruct> DefaultParams;

struct USPersonSolidShader : nglShader {
    void sub_41DEE0(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    void sub_41E0A0(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    void sub_41E290(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    //0x0041DBE0
    //virtual
    void _AddNode(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    //virtual
    void _BindMaterial(nglMaterialBase *a1);

    void _RebaseMaterial(nglMaterialBase *a1, unsigned int a2);

    //virtual
    void Register();
};

struct USPersonShader : nglShader {
    tlFixedString field_C;

    USPersonShader();

    //0x0041BEF0
    void AddNodeOverrideMask(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    //0x0041C0B0
    void AddNodeClearZ(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4);

    //0x0041C2A0
    void AddNodeExtraOutline(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3);

    //0x0041BBC0
    //virtual
    void _AddNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4);

    //virtual
    void _BindMaterial(nglMaterialBase *);

    //virtual
    void _RebaseMaterial(nglMaterialBase *a1, unsigned int a2);

    //0x00411580
    //virtual
    void _Register();

    //0x00410CC0
    //virtual
    tlFixedString _GetName();
};

struct USPersonSolidNode : USVariantShaderNode {
    USPersonMaterialSolid *field_18;
    nglTexture *field_1C;
    nglTexture *field_20;
    int field_24;

    //0x0041DCF0
    USPersonSolidNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4);

    void *operator new(size_t size);

    //0x0041E4B0
    /* virtual */ void _Render() /* override */;

    /* virtual */ void _GetSortInfo(nglSortInfo &sortInfo) /* override */;

    /* virtual */ ~USPersonSolidNode() = default;
};

struct USPersonNode : USVariantShaderNode {
    struct LightInfoStruct {
        vector4d m_dir;
        color field_10;
        color field_20;
        float m_contrast;
        uint32_t m_flags;
    };

    static Var<LightInfoStruct> DefaultLightInfo;

    USPersonMaterial *m_material;
    nglTexture *field_1C;
    nglTexture *field_20;
    int field_24;

    //0x0041BCD0
    USPersonNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4);

    void *operator new(size_t size);

    //0x0041CF70
    bool GetLightInfo(USPersonNode::LightInfoStruct &lightInfo);

    //0x0041D180
    void RenderWithDisableShader();

    //0x0041C4C0
    /* virtual */ void _Render() /* override */;

    //0x0041DD90
    /* virtual */ void _GetSortInfo(nglSortInfo &sortInfo) /* override */;

    /* virtual */ ~USPersonNode() = default;
};

}  // namespace USPersonShaderSpace

struct USPersonParam {
    USPersonShaderSpace::ParamStruct *field_0;

    static Var<int> ID;
};

extern void us_person_patch();
