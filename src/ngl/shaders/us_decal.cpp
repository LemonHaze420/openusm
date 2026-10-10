#include "us_decal.h"

#include "trace.h"
#include "utility.h"
#include "ngl.h"
#include "us_exterior.h"

#if STANDALONE_SYSTEM
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include <ngl_mesh.h>
#include <ngl_scene.h>
#include "nglsortinfo.h"
#include "variables.h"
#include <new>
#include <functional>
namespace {
// @todo - shaders
constexpr DWORD decal_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000001,
    0xE00F0000, 0x90E40001, 0x00000005, 0xD00F0000, 0x90C00002, 0xA0E40004, 0x0000FFFF,
};
struct DecalMaterial {
    uint8_t header[0x1C];
    vector4d colours[4];
    uint32_t tod_frames;
    tlFixedString *texture_name;
    nglTexture *texture;
    uint32_t mode;
};
static_assert(offsetof(DecalMaterial, mode) == 0x68);
VShader decal_program{};
IDirect3DPixelShader9 *decal_pixels[3]{};
IDirect3DVertexDeclaration9 *decal_fixed{};
constexpr D3DVERTEXELEMENT9 decal_elements[]{
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    D3DDECL_END(),
};
struct DecalNode : nglShaderNode {
    nglMaterialBase *material;
    nglTexture *texture;
    DecalNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *source)
        : nglShaderNode(mesh, section), material(source)
    {
        const auto &data = *reinterpret_cast<DecalMaterial *>(source);
        texture = data.texture;
        if ((texture->m_format & 0xFFu) == 16) {
            auto &params = mesh->field_8C;
            const uint32_t frame = params.IsSetParam<nglTextureFrameParam>()
                                       ? params.Get<nglTextureFrameParam>()->field_0
                                   : data.tod_frames ? uint32_t(g_TOD) + 4u * nglCurScene->IFLFrame
                                                     : nglCurScene->IFLFrame;
            texture = texture->Frames[frame % texture->m_num_palettes];
        }
        static void *table[]{
            func_address(&DecalNode::Render), func_address(&DecalNode::Sort), func_address(&DecalNode::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    DecalNode *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Sort(nglSortInfo &out)
    {
        out.Type = NGLSORT_OPAQUE;
        out.u = (3u * var<uint32_t>(0x91E534) + reinterpret_cast<DecalMaterial *>(material)->mode) | 0x40000000u;
    }
    void Render();
};
struct DecalShader : nglShader {
    static void __fastcall Name(DecalShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{"US_Decal3D"};
    }
    static bool __fastcall Switchable(DecalShader *, void *)
    {
        return true;
    }
    DecalShader()
    {
        static void *table[]{func_address(&DecalShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&DecalShader::Add),
                             func_address(&DecalShader::Bind),
                             func_address(&DecalShader::Release),
                             func_address(&DecalShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&DecalShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    DecalShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        if (EnableShader && !decal_program.field_0) {
            nglCreateVertexDeclarationAndShader(&decal_program, decal_elements, decal_vertex);
            nglCreatePShader(&decal_pixels[0], "mov r0, c0\n");
            nglCreatePShader(&decal_pixels[1], "mov r0, v0\n");
            nglCreatePShader(&decal_pixels[2], "tex t0\nmul r0, t0, v0\nmov r0.a, c0.a\n");
        } else if (!EnableShader && !decal_fixed)
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, decal_elements, &decal_fixed);
    }
    void Bind(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<DecalMaterial *>(material);
        data.texture = nglLoadTexture(*data.texture_name);
    }
    void Release(nglMaterialBase *material)
    {
        auto &texture = reinterpret_cast<DecalMaterial *>(material)->texture;
        nglReleaseTexture(texture);
        texture = nullptr;
    }
    void Rebase(nglMaterialBase *material, unsigned offset)
    {
        auto &name = reinterpret_cast<DecalMaterial *>(material)->texture_name;
        if (name)
            name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(name) + offset);
    }
    void Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
    {
        auto *node = new (nglListAlloc(sizeof(DecalNode), 16)) DecalNode{mesh, section, material};
        nglListAddNode(node);
    }
};
}
extern void sub_413850(nglMaterialBase *, nglParamSet<nglShaderParamSet_Pool> *, color *);
void DecalNode::Render()
{
    const auto &data = *reinterpret_cast<DecalMaterial *>(material);
    if (var<int>(0x956D48) || data.mode > 2)
        return;
    sub_413AF0();
    auto &state = g_renderState();
    const auto depth_function = state.field_7C;
    const bool depth_write = state.field_74;
    state.m_blend_mode = 9;
    state.field_D0 = state.field_D4 = -1;
    state.setCullingMode(data.mode == 0 ? D3DCULL_NONE : D3DCULL_CW);
    state.setAlphaBlending(data.mode != 0);
    state.setAlphaTesting(data.mode == 2);
    if (data.mode != 0)
        state.setAlphaReferenceValue(0);
    state.setColourBufferWriteEnabled(data.mode == 2 ? 7 : 0);
    state.setDepthBuffer(D3DZB_TRUE);
    state.setDepthBufferFunction(data.mode == 1 ? D3DCMP_ALWAYS : D3DCMP_LESSEQUAL);
    if (data.mode != 2) {
        nglSetDepthBias(0);
        state.setDepthBufferWriteEnabled(data.mode == 1);
        state.setStencilCheckEnabled(true);
        state.setStencilDepthFailOperation(data.mode == 1 ? D3DSTENCILOP_ZERO : D3DSTENCILOP_KEEP);
        state.setStencilFailOperation(D3DSTENCILOP_KEEP);
        state.setStencilRefValue(1);
        state.setStencilBufferTestFunction(data.mode == 1 ? D3DCMP_EQUAL : D3DCMP_ALWAYS);
        if (data.mode == 1)
            state.setStencilBufferCompareMask(1);
        state.setStencilBufferWriteMask(1);
        state.setStencilPassOperation(data.mode == 1 ? D3DSTENCILOP_ZERO : D3DSTENCILOP_REPLACE);
    } else {
        state.setBlendOperation(D3DBLENDOP_ADD);
        nglDxSetTexture(0, texture, 8, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    }
    if (EnableShader) {
        nglSetVertexDeclarationAndShader(&decal_program);
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
        SetPixelShader(&decal_pixels[data.mode]);
        if (data.mode == 0) {
            const float zero[4]{};
            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, zero, 1);
        }
    } else {
        IDirect3DDevice9_SetTransform(
            g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, decal_fixed);
        if (data.mode == 0)
            state.setBlendingFactor(0);
        nglSetTextureStageState(0, D3DTSS_COLOROP, data.mode == 2 ? D3DTOP_MODULATE : D3DTOP_SELECTARG1);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, data.mode == 0 ? D3DTA_TFACTOR : D3DTA_DIFFUSE);
        if (data.mode == 2)
            nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(0,
                                D3DTSS_ALPHAARG1,
                                data.mode == 2   ? D3DTA_TFACTOR
                                : data.mode == 0 ? D3DTA_TFACTOR
                                                 : D3DTA_DIFFUSE);
        nglSetTextureStageState(1, D3DTSS_COLOROP, data.mode == 2 ? D3DTOP_MODULATE : D3DTOP_DISABLE);
        if (data.mode == 2) {
            nglSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            nglSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
            nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        } else
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    if (data.mode == 2) {
        color value;
        sub_413850(material, &m_meshNode->field_8C, &value);
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &value.r, 1);
            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, &value.r, 1);
        } else {
            state.setBlendingFactor((uint32_t(value.a * 255) << 24) | ((uint32_t(value.r * 255) & 255) << 16) |
                                    ((uint32_t(value.g * 255) & 255) << 8) | (uint32_t(value.b * 255) & 255));
        }
        state.setBlending(std::not_equal_to<float>{}(value.a, 1.0f) ? NGLBM_BLEND : NGLBM_OPAQUE, 0, 128);
        state.setColourBufferWriteEnabled(7);
    }
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    if (data.mode == 1) {
        state.setStencilDepthFailOperation(D3DSTENCILOP_KEEP);
        state.setStencilRefValue(1);
        state.setStencilBufferCompareMask(~0u);
        state.setStencilBufferWriteMask(~0u);
        state.setStencilPassOperation(D3DSTENCILOP_KEEP);
        state.setAlphaBlending(false);
        state.setAlphaTesting(false);
    }
    if (data.mode != 2) {
        state.setStencilCheckEnabled(false);
        state.setColourBufferWriteEnabled(7);
    }
    state.setDepthBufferFunction(depth_function);
    state.setDepthBufferWriteEnabled(depth_write);
}
void initialize_decal_material_shader()
{
    static DecalShader shader;
}
#endif

void US_Decal3DShader::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("US_Decal3DShader::BindMaterial");

    struct {
        char field_0[0x60];
        tlFixedString *field_60;
        nglTexture *field_64;
    } *v1 = CAST(v1, a1);

#ifdef TARGET_XBOX
    v1->field_64 = nglLoadTexture(*bit_cast<tlHashString *>(&v1->field_60));
#else
    v1->field_64 = nglLoadTexture(*bit_cast<tlFixedString *>(v1->field_60));
#endif
}

void US_Decal3DShader::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("US_Decal3DShader::RebaseMaterial");

    USExteriorMaterial *v1 = CAST(v1, a1);

#ifndef TARGET_XBOX
    auto v2 = v1->field_60;
    if (v2 != nullptr) {
        v1->field_60 = CAST(v1->field_60, int(v2) + a2);
    }
#endif
}

void us_decal_patch()
{
    {
        FUNC_ADDRESS(address, &US_Decal3DShader::_BindMaterial);
        set_vfunc(0x00870CA0, address);
    }

    {
        FUNC_ADDRESS(address, &US_Decal3DShader::_RebaseMaterial);
        set_vfunc(0x00870CA8, address);
    }
}
