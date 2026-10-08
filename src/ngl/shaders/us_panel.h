#pragma once

#include "nglshader.h"
#include "ngl.h"

struct USPanelShader : nglShader {
    USPanelShader();
    void Register();
    void AddPanelNode(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
    void BindPanelMaterial(nglMaterialBase *);
    void ReleasePanelMaterial(nglMaterialBase *);
    void RebasePanelMaterial(nglMaterialBase *, unsigned);
    void CopyTexture(nglTexture *source, nglTexture *destination, const RECT &rect);
};

struct USPanelShaderMaterial {
    std::intptr_t m_vtbl;
    nglMeshSection *section;
    USPanelShader *shader;
    uint32_t reserved[4];
    tlFixedString *texture_name;
    nglTexture *texture;
    tlFixedString *secondary_name;
    nglTexture *secondary;
    nglBlendModeType blend;
    uint32_t flags;
    uint32_t field_34;
    uint32_t field_38;
    int mode;

    USPanelShaderMaterial();
    nglMaterialBase *material()
    {
        return reinterpret_cast<nglMaterialBase *>(&section);
    }
};

struct USPanelShaderNode : nglShaderNode {
    USPanelShaderMaterial *material;
    USPanelShaderNode(nglMeshNode *, nglMeshSection *, USPanelShaderMaterial *);
    void Render();
};

USPanelShader &getUSPanelShader();
