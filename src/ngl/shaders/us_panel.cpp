#include "us_panel.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl_scene.h"
#include "ngl_mesh.h"
#include "ngl_vertexdef.h"
#include "variables.h"
#include "vtbl.h"
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include <new>

VALIDATE_SIZE(USPanelShaderMaterial, 0x40);
VALIDATE_SIZE(USPanelShaderNode, 0x18);

namespace {
VShader panel_vertex_shader{};
IDirect3DPixelShader9 *panel_pixel_shaders[7]{};
void __fastcall register_panel_shader(USPanelShader *self, void *)
{
    self->Register();
}
void __fastcall panel_shader_name(USPanelShader *, void *, tlFixedString *out)
{
    *out = tlFixedString{"Panel"};
}
void __fastcall add_panel_node(USPanelShader *self, void *, nglMeshNode *node, nglMeshSection *section,
                               nglMaterialBase *material)
{
    self->AddPanelNode(node, section, material);
}
void __fastcall bind_panel_material(USPanelShader *self, void *, nglMaterialBase *material)
{
    self->BindPanelMaterial(material);
}
void __fastcall release_panel_material(USPanelShader *self, void *, nglMaterialBase *material)
{
    self->ReleasePanelMaterial(material);
}
void __fastcall rebase_panel_material(USPanelShader *self, void *, nglMaterialBase *material, unsigned offset)
{
    self->RebasePanelMaterial(material, offset);
}
bool __fastcall panel_not_switchable(USPanelShader *, void *)
{
    return false;
}
void __fastcall render_panel_node(USPanelShaderNode *self, void *)
{
    self->Render();
}
void __fastcall destroy_panel_material(USPanelShaderMaterial *self, void *, bool release)
{
    if (release)
        ::operator delete(self);
}
USPanelShaderMaterial *panel_material(nglMaterialBase *material)
{
    return reinterpret_cast<USPanelShaderMaterial *>(reinterpret_cast<char *>(material) - sizeof(std::intptr_t));
}
}

USPanelShader::USPanelShader()
{
    static void *table[]{reinterpret_cast<void *>(register_panel_shader),
                         reinterpret_cast<void *>(panel_shader_name),
                         reinterpret_cast<void *>(add_panel_node),
                         reinterpret_cast<void *>(bind_panel_material),
                         reinterpret_cast<void *>(release_panel_material),
                         reinterpret_cast<void *>(rebase_panel_material),
                         func_address(&nglShader::_CheckMaterialVersion),
                         func_address(&nglShader::_CheckVertexDefVersion),
                         func_address(&nglShader::_BindSection),
                         reinterpret_cast<void *>(panel_not_switchable)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
}

USPanelShader &getUSPanelShader()
{
    static USPanelShader shader;
    return shader;
}


USPanelShaderMaterial::USPanelShaderMaterial()
    : section(nullptr), shader(&getUSPanelShader()), reserved{}, texture_name(nullptr), texture(nullptr),
      secondary_name(nullptr), secondary(nullptr), blend(NGLBM_OPAQUE), flags(2), field_34(0), field_38(0), mode(0)
{
    static void *table[]{reinterpret_cast<void *>(destroy_panel_material)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
}


void USPanelShader::Register()
{
    nglShader::_Register();
    static D3DVERTEXELEMENT9 declaration[]{{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
                                           {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                                           {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
                                           D3DDECL_END()};
    if (EnableShader) {
        auto mesh_color_vertex_code = CompileVShader("shaders/us_frontend_VS.hlsl");
        nglCreateVertexDeclarationAndShader(&panel_vertex_shader, declaration, mesh_color_vertex_code.data());
        auto particle_ps_code = CompilePShader("shaders/us_frontend_PS.hlsl");
        auto texture_pixels_code = CompilePShader("shaders/texture_pixels.hlsl");
        auto panel_gb_lookup_pixels_code = CompilePShader("shaders/panel_gb_lookup_pixels.hlsl");
        auto panel_dot_pixels_code = CompilePShader("shaders/panel_dot_pixels.hlsl");
        auto panel_ar_lookup_pixels_code = CompilePShader("shaders/panel_ar_lookup_pixels.hlsl");
        const DWORD *programs[]{particle_ps_code.data(),
                                texture_pixels_code.data(),
                                particle_ps_code.data(),
                                particle_ps_code.data(),
                                panel_gb_lookup_pixels_code.data(),
                                panel_dot_pixels_code.data(),
                                panel_ar_lookup_pixels_code.data()};
        for (unsigned index = 0; index != 7; ++index)
            CreatePixelShader(&panel_pixel_shaders[index], programs[index]);
    } else if (!dword_9738E0[21]) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, declaration, &dword_9738E0[21]);
    }
}


void USPanelShader::AddPanelNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *base)
{
    auto *material = panel_material(base);
    auto *node = new (nglListAlloc(sizeof(USPanelShaderNode), 16)) USPanelShaderNode{mesh, section, material};
    if (material->blend <= NGLBM_PUNCHTHROUGH) {
        node->m_tex = reinterpret_cast<nglTexture *>(static_cast<uint32_t>(material->shader->field_8) << 24);
        node->m_next_node = nglCurScene->OpaqueNodes;
        nglCurScene->OpaqueNodes = node;
        ++nglCurScene->OpaqueListCount;
    } else {
        sub_417C10(node);
    }
}

void USPanelShader::BindPanelMaterial(nglMaterialBase *base)
{
    auto *material = panel_material(base);
    material->texture = nglLoadTexture(*material->texture_name);
    material->secondary = nglLoadTexture(*material->secondary_name);
}
void USPanelShader::ReleasePanelMaterial(nglMaterialBase *base)
{
    auto *material = panel_material(base);
    nglReleaseTexture(material->texture);
    material->texture = nullptr;
    nglReleaseTexture(material->secondary);
    material->secondary = nullptr;
}
void USPanelShader::RebasePanelMaterial(nglMaterialBase *base, unsigned offset)
{
    auto *material = panel_material(base);
    if (material->texture_name)
        material->texture_name =
            reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(material->texture_name) + offset);
    if (material->secondary_name)
        material->secondary_name =
            reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(material->secondary_name) + offset);
}

USPanelShaderNode::USPanelShaderNode(nglMeshNode *mesh, nglMeshSection *section, USPanelShaderMaterial *data)
    : nglShaderNode(mesh, section), material(data)
{
    static void *table[]{reinterpret_cast<void *>(render_panel_node)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
}


void USPanelShaderNode::Render()
{
    static Var<int> disabled{0x00956978};
    if (disabled())
        return;
    if (g_distance_clipping_enabled)
        g_renderState().setFogEnable(false);
    g_renderState().setCullingMode(D3DCULL_NONE);
    g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
        nglSetVertexDeclarationAndShader(&panel_vertex_shader);
    } else {
        IDirect3DDevice9_SetTransform(g_Direct3DDevice,
                                      static_cast<D3DTRANSFORMSTATETYPE>(256),
                                      reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[21]);
    }
    g_renderState().setBlending(material->blend, material->field_34, 0);
    nglDxSetTexture(0, material->texture, material->flags, 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, ((material->flags & 0x40u) | 0x20u) >> 5);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, ((material->flags & 0x80u) | 0x40u) >> 6);
    if (material->mode == 4) {
        nglDxSetTexture(1, material->secondary, material->flags, 3);
        nglSetSamplerState(1, D3DSAMP_ADDRESSU, 3);
        nglSetSamplerState(1, D3DSAMP_ADDRESSV, 3);
    }
    if (EnableShader) {
        SetPixelShader(&panel_pixel_shaders[material->mode]);
        if (material->mode == 5) {
            const float weights[]{0.299f, 0.587f, 0.114f, 1.0f};
            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, weights, 1);
        }
    } else {
        const int mode = material->mode;
        if (mode == 0 || mode == 2 || mode == 3 || mode == 5) {
            if (mode == 5)
                g_renderState().setBlendingFactor(0xFF4D4D4D);
            nglSetTextureStageState(0, D3DTSS_COLOROP, mode == 5 ? 24 : 4);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, 0);
            nglSetTextureStageState(0, D3DTSS_COLORARG2, 2);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, mode == 5 ? 24 : 4);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 0);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG2, 2);
        } else if (mode == 1 || mode == 4 || mode == 6) {
            nglSetTextureStageState(0, D3DTSS_COLOROP, 2);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, mode == 1 ? 2 : 0);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, 2);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, mode == 1 ? 2 : 0);
        }
        if (mode >= 0 && mode <= 6) {
            nglSetTextureStageState(1, D3DTSS_COLOROP, 1);
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, 1);
        }
    }
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    if (g_distance_clipping_enabled)
        g_renderState().setFogEnable(true);
}

void USPanelShader::CopyTexture(nglTexture *source, nglTexture *destination, const RECT &rect)
{
    const bool write = g_renderState().field_74;
    if (g_renderState().field_B0 != D3DFILL_SOLID) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
        g_renderState().field_B0 = D3DFILL_SOLID;
    }
    g_renderState().setCullingMode(D3DCULL_NONE);
    g_renderState().setDepthBuffer(D3DZB_FALSE);
    g_renderState().setDepthBufferWriteEnabled(false);
    g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
    matrix4x4 identity{identity_matrix};
    if (EnableShader) {
        nglSetVertexDeclarationAndShader(&panel_vertex_shader);
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &identity[0][0], 4);
        SetPixelShader(&panel_pixel_shaders[1]);
    } else {
        IDirect3DDevice9_SetTransform(
            g_Direct3DDevice, static_cast<D3DTRANSFORMSTATETYPE>(256), reinterpret_cast<const D3DMATRIX *>(&identity));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[21]);
        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    nglDxSetTexture(0, source, 1, 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    IDirect3DDevice9_SetRenderTarget(g_Direct3DDevice, 0, destination->DXSurfaces[0]);
    struct vertex {
        float x, y, z, u, v;
        uint32_t color;
    };
    const float x0 = 2.0f * rect.left / destination->m_width - 1.0f;
    const float x1 = 2.0f * rect.right / destination->m_width - 1.0f;
    const float y0 = 2.0f * rect.top / destination->m_height - 1.0f;
    const float y1 = 2.0f * rect.bottom / destination->m_height - 1.0f;
    const float u0 = (rect.left + .5f) / source->m_width;
    const float u1 = (rect.right + .5f) / source->m_width;
    const float v0 = (rect.top + .5f) / source->m_height;
    const float v1 = (rect.bottom + .5f) / source->m_height;
    const vertex vertices[]{{x0, y1, 0, u0, v0, 0xFFFFFFFFu},
                            {x1, y1, 0, u1, v0, 0xFFFFFFFFu},
                            {x0, y0, 0, u0, v1, 0xFFFFFFFFu},
                            {x1, y0, 0, u1, v1, 0xFFFFFFFFu}};
    IDirect3DDevice9_DrawPrimitiveUP(g_Direct3DDevice, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(vertex));
    g_renderState().setDepthBufferWriteEnabled(write);
    g_renderState().setDepthBuffer(D3DZB_TRUE);
    SetRenderTarget(nglCurScene->field_334, nglCurScene->field_338, 0, 6);
}
