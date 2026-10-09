#include "gen_building.h"

#include "common.h"
#include "color.h"
#include "procedural_building_system.h"
#include "func_wrapper.h"
#include "lego_map.h"
#include "ngl.h"
#include "nglshader.h"
#include "nglsortinfo.h"
#include "oldmath_po.h"
#include "region.h"
#include "variables.h"

#include <ngl_params.h>
#include <ngl_scene.h>
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include <ngl_mesh.h>
#include "utility.h"
#include <array>
#include <new>
#include <functional>

extern void sub_413850(nglMaterialBase *, nglParamSet<nglShaderParamSet_Pool> *, color *);

namespace {
struct ProcBlgNode {
    matrix4x4 local_to_world;
    nglParamSet<nglShaderParamSet_Pool> params;
    const scene_entity *building;
};
static_assert(sizeof(ProcBlgNode) == 0x48);

struct ProcMaterial {
    uint8_t header[0x1C];
    color time_of_day_colour[4];
    uint32_t time_of_day_animation;
    uint32_t texture_name;
    nglTexture *texture;
    uint32_t field_68;
    uint32_t field_6C;
    nglBlendModeType blend_mode;
};
static_assert(offsetof(ProcMaterial, time_of_day_animation) == 0x5C);
static_assert(offsetof(ProcMaterial, texture) == 0x64);
static_assert(offsetof(ProcMaterial, blend_mode) == 0x70);

#if STANDALONE_SYSTEM
Var<int> procedural_render_disabled{0x00956FA0};

struct ProcVertex {
    float x, y, z, u, v;
    uint32_t colour;
};
static_assert(sizeof(ProcVertex) == 24);

ProcVertex vertex(float x, float y, float z, float u, float v)
{
    return {x, y, z, u, v, 0xFFFFFFFFu};
}

void quad(ProcVertex *out, ProcVertex a, ProcVertex b, ProcVertex c, ProcVertex d)
{
    out[0] = a;
    out[1] = b;
    out[2] = c;
    out[3] = c;
    out[4] = b;
    out[5] = d;
}


constexpr DWORD building_vs[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E4000B, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E4000C,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E4000D, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E4000E, 0x00000001,
    0xE0030000, 0x90E40001, 0x00000009, 0x80020000, 0x90E40000, 0xA0E40001, 0x00000002, 0x80020002, 0x80550000,
    0xA0550009, 0x00000005, 0xE0030002, 0x80550002, 0xA0000009, 0x00000005, 0xD0070000, 0x90000002, 0xA0E40008,
    0x00000001, 0xD0080000, 0xA0E40008, 0x0000FFFF,
};
constexpr DWORD building_ps[] = {
    0xFFFF0101,
    0x00000042,
    0xB00F0000,
    0x00000042,
    0xB00F0002,
    0x00000005,
    0x800F0000,
    0x90E40000,
    0xB0E40002,
    0x00000005,
    0x80070000,
    0x80E40000,
    0xB0E40000,
    0x0000FFFF,
};
constexpr DWORD windows_vs[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000001, 0x80070000, 0x90E40000, 0x00000001, 0x80080000, 0xA0AA005B, 0x00000009, 0xC0010000,
    0x80E40000, 0xA0E4000B, 0x00000009, 0xC0020000, 0x80E40000, 0xA0E4000C, 0x00000009, 0xC0040000, 0x80E40000,
    0xA0E4000D, 0x00000009, 0xC0080000, 0x80E40000, 0xA0E4000E, 0x00000001, 0xE00F0000, 0x90E40001, 0x00000005,
    0xE00F0001, 0x90E40001, 0xA0E4000F, 0x00000009, 0x80020002, 0x90E40000, 0xA0E40001, 0x00000002, 0x80020003,
    0x80550002, 0xA0550009, 0x00000005, 0xE0030002, 0x80550003, 0xA0000009, 0x00000002, 0x80020003, 0x80550002,
    0xA055000A, 0x00000005, 0xE0030003, 0x80550003, 0xA000000A, 0x00000005, 0xD0070000, 0x90000002, 0xA0E40008,
    0x00000001, 0xD0080000, 0xA0E40008, 0x0000FFFF,
};
constexpr DWORD windows_ps[] = {
    0xFFFF0101, 0x00000042, 0xB00F0000, 0x00000042, 0xB00F0001, 0x00000042, 0xB00F0002, 0x00000042, 0xB00F0003,
    0x00000005, 0x800F0000, 0x90E40000, 0xB0E40002, 0x00000005, 0x80070000, 0x80E40000, 0xB0E40000, 0x00000005,
    0x800F0001, 0xB0E40001, 0xB0E40003, 0x00000012, 0x80070000, 0xB0FF0000, 0x80E40000, 0x80E40001, 0x0000FFFF,
};
constexpr DWORD roof_vs[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000002,
    0xE0030000, 0x90E40001, 0xA0540005, 0x00000005, 0xD00F0000, 0x90C00002, 0xA0E40004, 0x0000FFFF,
};
constexpr DWORD roof_ps[] = {
    0xFFFF0101,
    0x00000042,
    0xB00F0000,
    0x00000005,
    0x800F0000,
    0xB0E40000,
    0x90E40000,
    0x0000FFFF,
};

struct ProcShaders {
    VShader building{}, windows{}, roof{};
    IDirect3DPixelShader9 *building_pixel{}, *windows_pixel{}, *roof_pixel{};
    IDirect3DVertexDeclaration9 *fixed_declaration{};
};

ProcShaders &shaders(bool procedural = true)
{
    static ProcShaders resources;
    static const D3DVERTEXELEMENT9 elements[] = {
        {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
        {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
        {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
        D3DDECL_END(),
    };
    if (EnableShader) {
        if (!resources.building.field_0) {
            nglCreateVertexDeclarationAndShader(&resources.building, elements, building_vs);
            CreatePixelShader(&resources.building_pixel, building_ps);
        }
        if (procedural && !resources.windows.field_0) {
            nglCreateVertexDeclarationAndShader(&resources.windows, elements, windows_vs);
        }
        if (!resources.windows_pixel)
            CreatePixelShader(&resources.windows_pixel, windows_ps);
        if (procedural && !resources.roof.field_0) {
            nglCreateVertexDeclarationAndShader(&resources.roof, elements, roof_vs);
            CreatePixelShader(&resources.roof_pixel, roof_ps);
        }
    } else if (!resources.fixed_declaration) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, elements, &resources.fixed_declaration);
    }
    return resources;
}

nglTexture *material_texture(nglMaterialBase *material, nglParamSet<nglShaderParamSet_Pool> &params,
                             nglTexture *texture)
{
    const auto &data = *reinterpret_cast<const ProcMaterial *>(material);
    if ((texture->m_format & 0xFFu) != 16u)
        return texture;
    uint32_t frame;
    if (params.IsSetParam<nglTextureFrameParam>()) {
        frame = params.Get<nglTextureFrameParam>()->field_0;
    } else if (data.time_of_day_animation) {
        frame = static_cast<uint32_t>(g_TOD + 4 * nglCurScene->IFLFrame);
    } else {
        frame = nglCurScene->IFLFrame;
    }
    return texture->Frames[frame % texture->m_num_palettes];
}


void material_colour(nglMaterialBase *material, nglParamSet<nglShaderParamSet_Pool> &params, unsigned reg,
                     bool preserve_blend = true)
{
    color value;
    sub_413850(material, &params, &value);
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, reg, &value.r, 1);
    } else {
        const uint32_t packed =
            (static_cast<uint32_t>(value.a * 255.0f) << 24) | ((static_cast<uint32_t>(value.r * 255.0f) & 255u) << 16) |
            ((static_cast<uint32_t>(value.g * 255.0f) & 255u) << 8) | (static_cast<uint32_t>(value.b * 255.0f) & 255u);
        if (g_renderState().field_9C != packed) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed);
            g_renderState().field_9C = packed;
        }
    }
    auto blend = preserve_blend ? static_cast<uint32_t>(reinterpret_cast<const ProcMaterial *>(material)->blend_mode)
                                : static_cast<uint32_t>(NGLBM_OPAQUE);
    if (std::not_equal_to<float>{}(value.a, 1.0f) && blend <= 1u)
        blend = NGLBM_BLEND;
    g_renderState().setBlending(static_cast<nglBlendModeType>(blend), 0, 128);
    g_renderState().setColourBufferWriteEnabled(7);
}

void building_material(nglMaterialBase *material, const matrix4x4 &local_to_world,
                       nglParamSet<nglShaderParamSet_Pool> &params, bool preserve_blend = true)
{
    static nglTexture *lighting[4]{};
    static const char *names[] = {"us_day_light", "us_night_light", "us_rainy_light", "us_sunset_light"};
    if (!lighting[g_TOD])
        lighting[g_TOD] = nglLoadTexture(tlFixedString{names[g_TOD]});
    if (EnableShader) {
        const auto transform = local_to_world.transpose();
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &transform[0][0], 3);
    }
    material_colour(material, params, 8, preserve_blend);
    if (EnableShader) {
        const float constants[] = {0.02f, 0.0f, 4.0f, 3.0f};
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 9, constants, 1);
        nglDxSetTexture(2, lighting[g_TOD], 8, 3);
        nglSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        nglSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
}

void window_material()
{
    static nglTexture *lighting[4]{};
    static const char *names[] = {
        "us_day_light_window", "us_night_light_window", "us_rainy_light_window", "us_sunset_light_window"};
    if (!lighting[g_TOD])
        lighting[g_TOD] = nglLoadTexture(tlFixedString{names[g_TOD]});
    if (EnableShader) {
        const float constants[] = {0.01f, 0.0f, 4.0f, 3.0f};
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 10, constants, 1);
        nglDxSetTexture(3, lighting[g_TOD], 8, 3);
        nglSetSamplerState(3, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        nglSetSamplerState(3, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
}

void depth_bias(float amount)
{
    const float value = -0.00001f * amount;
    if (std::not_equal_to<float>{}(g_renderState().field_94, value)) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_SLOPESCALEDEPTHBIAS, bit_cast<uint32_t>(value));
        g_renderState().field_94 = value;
    }
    if (std::not_equal_to<float>{}(g_renderState().field_98, value)) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_DEPTHBIAS, bit_cast<uint32_t>(value));
        g_renderState().field_98 = value;
    }
}

void bind_building_shader(ProcShaders &resources)
{
    if (EnableShader) {
        nglSetVertexDeclarationAndShader(&resources.building);
        SetPixelShader(&resources.building_pixel);
    } else {
        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
}

void draw_triangles(const ProcVertex *vertices, unsigned count)
{
    IDirect3DDevice9_DrawPrimitiveUP(g_Direct3DDevice, D3DPT_TRIANGLELIST, count, vertices, sizeof(ProcVertex));
}

constexpr uint16_t wall_indices[] = {0, 1, 2, 3, 1, 5, 3, 7, 5, 4, 7, 6, 4, 0, 6, 2};
constexpr uint16_t column_indices[] = {3,  2,  1,  0,  5,  4,  4,  9,  9,  8,  7,  6,  11, 10, 10,
                                       15, 15, 14, 13, 12, 17, 16, 16, 21, 21, 20, 19, 18, 23, 22};


constexpr DWORD mesh_windows_vs[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E4000B, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E4000C,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E4000D, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E4000E, 0x00000001,
    0xE00F0000, 0x90E40001, 0x00000005, 0xE00F0001, 0xA0E4000F, 0x90E40001, 0x00000009, 0x80020000, 0x90E40000,
    0xA0E40001, 0x00000002, 0x80020002, 0x80550000, 0xA0550009, 0x00000005, 0xE0030002, 0x80550002, 0xA0000009,
    0x00000002, 0x80020002, 0x80550000, 0xA055000A, 0x00000005, 0xE0030003, 0x80550002, 0xA000000A, 0x00000005,
    0xD0070000, 0x90000002, 0xA0E40008, 0x00000001, 0xD0080000, 0xA0E40008, 0x0000FFFF,
};

VShader &mesh_window_program()
{
    static VShader program{};
    if (EnableShader && !program.field_0) {
        static const D3DVERTEXELEMENT9 elements[] = {
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END(),
        };
        nglCreateVertexDeclarationAndShader(&program, elements, mesh_windows_vs);
    }
    return program;
}

struct SimpleBuildingMaterial : ProcMaterial {
    uint32_t bias;
    uint32_t field_78;
    uint32_t cull;
};
static_assert(offsetof(SimpleBuildingMaterial, cull) == 0x7C);
static_assert(offsetof(SimpleBuildingMaterial, bias) == 0x74);

template <bool Windows, bool Glass = false>
struct BuildingMeshNode : nglShaderNode {
    nglMaterialBase *material;
    nglTexture *textures[Windows ? 2 : 1];

    BuildingMeshNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *source)
        : nglShaderNode(mesh, section), material(source)
    {
        const auto &data = *reinterpret_cast<const ProcMaterial *>(material);
        textures[0] = material_texture(material, mesh->field_8C, data.texture);
        if constexpr (Windows)
            textures[1] = material_texture(material, mesh->field_8C, reinterpret_cast<nglTexture *>(data.field_6C));
        static void *table[]{func_address(&BuildingMeshNode::Render),
                             func_address(&BuildingMeshNode::GetSortInfo),
                             func_address(&BuildingMeshNode::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }

    void GetSortInfo(nglSortInfo &) {}
    BuildingMeshNode *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Render()
    {
        if (var<int>(Glass ? 0x00956FA8 : Windows ? 0x00956FA4 : 0x00956D4C))
            return;
        const auto &data = *reinterpret_cast<const ProcMaterial *>(material);
        auto &state = g_renderState();
        auto &resources = shaders(false);
        if constexpr (Windows) {
            state.setCullingMode(D3DCULL_CW);
            state.setBlending(NGLBM_OPAQUE, 0, 0);
        } else {
            const auto &simple = *reinterpret_cast<const SimpleBuildingMaterial *>(material);
            state.setCullingMode(simple.cull == 2 ? D3DCULL_NONE : D3DCULL_CW);
            if (simple.bias)
                depth_bias(static_cast<float>(simple.bias + 1u));
            state.setBlending(data.blend_mode, 0, 0);
        }
        state.setColourBufferWriteEnabled(15);
        building_material(material, m_meshNode->LocalToWorld, m_meshNode->field_8C, !Windows);
        nglDxSetTexture(0, textures[0], Windows ? 4 : 8, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, !Windows && data.field_68 ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, !Windows && data.field_6C ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
        if constexpr (Windows) {
            window_material();
            nglDxSetTexture(1, textures[1], 4, 3);
            nglSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            nglSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        }
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 11, &m_meshNode->WorldToLocal[0][0], 4);
            if constexpr (Windows) {
                const float scale[]{0.2f, 0.2f, 1.0f, 1.0f};
                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 15, scale, 1);
                nglSetVertexDeclarationAndShader(&mesh_window_program());
                SetPixelShader(&resources.windows_pixel);
            } else {
                bind_building_shader(resources);
                const float constants[]{0.0f, 0.0f, 0.0f, 1.0f};
                IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, constants, 1);
            }
        } else {
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, resources.fixed_declaration);
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
            if constexpr (Windows) {
                if constexpr (!Glass) {
                    auto scale = identity_matrix;
                    scale[0][0] = scale[1][1] = 0.2f;
                    IDirect3DDevice9_SetTransform(
                        g_Direct3DDevice, D3DTS_TEXTURE1, reinterpret_cast<const D3DMATRIX *>(&scale));
                    nglSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
                    nglSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 0);
                }
                nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_BLENDCURRENTALPHA);
                nglSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
                nglSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
                nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
                nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            } else {
                bind_building_shader(resources);
            }
        }
        nglSetStreamSourceAndDrawPrimitive(m_meshSection);
        if constexpr (Windows && !Glass) {
            if (!EnableShader) {
                nglSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
                nglSetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
            }
        } else if constexpr (!Windows) {
            if (reinterpret_cast<const SimpleBuildingMaterial *>(material)->bias)
                depth_bias(0.0f);
        }
    }
};

template <bool Windows, bool Glass = false>
struct BuildingMeshShader : nglShader {
    static void __fastcall Name(BuildingMeshShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{Glass ? "USBuildingGlass" : Windows ? "USM_Building" : "USBuildingSimple"};
    }
    static bool __fastcall Switchable(BuildingMeshShader *, void *)
    {
        return true;
    }
    BuildingMeshShader()
    {
        static void *table[]{func_address(&BuildingMeshShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&BuildingMeshShader::Add),
                             func_address(&BuildingMeshShader::Bind),
                             func_address(&BuildingMeshShader::Release),
                             func_address(&BuildingMeshShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&BuildingMeshShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    BuildingMeshShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        shaders(false);
        if constexpr (Windows)
            mesh_window_program();
    }
    void Bind(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<ProcMaterial *>(material);
        data.texture = nglLoadTexture(*reinterpret_cast<tlFixedString *>(data.texture_name));
        if constexpr (Windows)
            data.field_6C =
                reinterpret_cast<uint32_t>(nglLoadTexture(*reinterpret_cast<tlFixedString *>(data.field_68)));
    }
    void Release(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<ProcMaterial *>(material);
        nglReleaseTexture(data.texture);
        data.texture = nullptr;
        if constexpr (Windows) {
            nglReleaseTexture(reinterpret_cast<nglTexture *>(data.field_6C));
            data.field_6C = 0;
        }
    }
    void Rebase(nglMaterialBase *material, unsigned int offset)
    {
        auto &data = *reinterpret_cast<ProcMaterial *>(material);
        if (data.texture_name)
            data.texture_name += offset;
        if constexpr (Windows) {
            if (data.field_68)
                data.field_68 += offset;
        }
    }
    void Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
    {
        using Node = BuildingMeshNode<Windows, Glass>;
        auto *node = new (nglListAlloc(sizeof(Node), 16)) Node{mesh, section, material};
        auto &params = mesh->field_8C;
        if (params.IsSetParam<nglTintParam>() &&
            std::not_equal_to<float>{}(params.Get<nglTintParam>()->field_0->w, 1.0f)) {
            sub_417C10(node);
        } else {
            node->m_tex = reinterpret_cast<nglTexture *>(uint32_t(field_8) << 24);
            node->m_next_node = nglCurScene->OpaqueNodes;
            nglCurScene->OpaqueNodes = node;
            ++nglCurScene->OpaqueListCount;
        }
    }
};
template <unsigned Kind>
struct ProceduralMaterialShader : nglShader {
    static void __fastcall Name(ProceduralMaterialShader *, void *, tlFixedString *out)
    {
        static const char *names[]{"USShadowVol", "SMLowlod", "SMProcBlg"};
        *out = tlFixedString{names[Kind]};
    }
    static bool __fastcall Switchable(ProceduralMaterialShader *, void *)
    {
        return true;
    }
    ProceduralMaterialShader()
    {
        static void *table[]{func_address(&ProceduralMaterialShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&ProceduralMaterialShader::Add),
                             func_address(&ProceduralMaterialShader::Material),
                             func_address(&ProceduralMaterialShader::Material),
                             func_address(&ProceduralMaterialShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&ProceduralMaterialShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    ProceduralMaterialShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        if constexpr (Kind == 1)
            shaders(false);
    }

    void Add(nglMeshNode *, nglMeshSection *, nglMaterialBase *) {}
    void Material(nglMaterialBase *) {}
    void Rebase(nglMaterialBase *, unsigned) {}
};
#endif
}  // namespace

#if STANDALONE_SYSTEM
void initialize_building_mesh_shaders()
{
    static BuildingMeshShader<false> simple;
    static BuildingMeshShader<true> windowed;
    static BuildingMeshShader<true, true> glass;
    static ProceduralMaterialShader<0> shadow;
    static ProceduralMaterialShader<1> lowlod;
    static ProceduralMaterialShader<2> procedural;
}
#endif

void render_generated_building(const scene_entity &building, Float fade, Float, region &owner,
                               const matrix4x4 &local_to_world)
{
    if (!(fade > 0.0f))
        return;
    nglParamSet<nglShaderParamSet_Pool> params{static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};
    if (fade < 1.0f) {
        auto *tint = new (nglListAlloc(sizeof(vector4d), 16)) vector4d{1.0f, 1.0f, 1.0f, fade};
        params.SetParam(nglTintParam{tint});
    }
    params.SetParam(USMMaterialListParam{owner.field_9C->field_4});
    USProcBlgTopAdd(&building, &local_to_world, &params);
}

void USProcBlgTopAdd(const scene_entity *building, const matrix4x4 *local_to_world,
                     nglParamSet<nglShaderParamSet_Pool> *params)
{
#if STANDALONE_SYSTEM
    auto *node = new (nglListAlloc(sizeof(ProcBlgNode), 16)) ProcBlgNode{*local_to_world, *params, building};
    nglSortInfo sort;
    sort.Type = NGLSORT_OPAQUE;
    sort.Dist = 0.0f;
    nglListAddCustomNode(USProcBlgTopRender, node, &sort);
#else
    CDECL_CALL(0x0040CAE0, building, local_to_world, params);
#endif
}

void USProcBlgTopRender(void *payload, void *)
{
#if STANDALONE_SYSTEM
    if (procedural_render_disabled())
        return;
    auto &node = *static_cast<ProcBlgNode *>(payload);
    const auto &record = *reinterpret_cast<const procedural_building_record *>(node.building);
    const float width = record.width, depth = record.depth, height = record.height;
    const float x = width * 2.5f, y = height * 2.5f, z = depth * 2.5f;
    const uint8_t flags = record.facade_flags;
    const float inset = (flags & 0x10) ? 0.3f : 0.0f;
    const float parapet_height = (flags & 0x40) ? 1.0f : 0.0f;
    const float uv_extra = (flags & 0x20) ? 0.2f : 0.0f;
    nglMaterialBase *materials[9];
    nglTexture *textures[9];
    auto **list = node.params.Get<USMMaterialListParam>()->field_0;
    for (unsigned i = 0; i < 9; ++i) {
        materials[i] = list[record.materials[i]];
        textures[i] =
            material_texture(materials[i], node.params, reinterpret_cast<const ProcMaterial *>(materials[i])->texture);
    }
    auto &resources = shaders();
    g_renderState().setCullingMode(D3DCULL_CW);
    g_renderState().setColourBufferWriteEnabled(7);
    g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
    depth_bias(0.0f);
    IDirect3DDevice9_SetStreamSource(g_Direct3DDevice, 0, nullptr, 0, 0);


    const auto projected = sub_507130(ptr_to_po{&node.local_to_world, &nglCurScene->WorldToScreen});
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 11, &projected[0][0], 4);
    } else {
        IDirect3DDevice9_SetTransform(
            g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&node.local_to_world));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, resources.fixed_declaration);
    }
    ProcVertex vertices[576];
    vertices[0] = vertex(x, -y, -z, -uv_extra, height);
    vertices[1] = vertex(x, -y, z, depth + uv_extra, height);
    vertices[2] = vertex(x, y, -z, -uv_extra, 0);
    vertices[3] = vertex(x, y, z, depth + uv_extra, 0);
    vertices[4] = vertex(-x, -y, -z, depth + uv_extra, height);
    vertices[5] = vertex(-x, -y, z, -uv_extra, height);
    vertices[6] = vertex(-x, y, -z, depth + uv_extra, 0);
    vertices[7] = vertex(-x, y, z, -uv_extra, 0);
    constexpr uint8_t wall_flags[] = {8, 1, 2, 4};
    for (unsigned wall = 0; wall < 4; ++wall) {
        if (flags & wall_flags[wall]) {
            building_material(materials[0], node.local_to_world, node.params);
            nglDxSetTexture(0, textures[0], 8, 3);
            bind_building_shader(resources);
        } else {
            building_material(materials[6], node.local_to_world, node.params);
            nglDxSetTexture(0, textures[6], 8, 3);
            window_material();
            nglDxSetTexture(1, textures[8], 8, 3);
            nglSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            nglSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            if (EnableShader) {
                const float scale[] = {0.2f, 0.2f, 1.0f, 1.0f};
                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 15, scale, 1);
                nglSetVertexDeclarationAndShader(&resources.windows);
                SetPixelShader(&resources.windows_pixel);
            } else {
                auto scale = identity_matrix;
                scale[0][0] = scale[1][1] = 0.2f;
                IDirect3DDevice9_SetTransform(
                    g_Direct3DDevice, D3DTS_TEXTURE1, reinterpret_cast<const D3DMATRIX *>(&scale));
                nglSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
                nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
                nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                nglSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
                nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
                nglSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
                nglSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
                nglSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
                nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            }
        }
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        IDirect3DDevice9_DrawIndexedPrimitiveUP(g_Direct3DDevice,
                                                D3DPT_TRIANGLESTRIP,
                                                0,
                                                8,
                                                2,
                                                wall_indices + 4 * wall,
                                                D3DFMT_INDEX16,
                                                vertices,
                                                sizeof(ProcVertex));
    }
    bind_building_shader(resources);
    if (flags & 0x40) {
        const float top = y + parapet_height, du = depth * 6.0f, wu = width * 6.0f;
        quad(vertices,
             vertex(x, y, -z, du, 1),
             vertex(x, y, z, 0, 1),
             vertex(x, top, -z, du, 0),
             vertex(x, top, z, 0, 0));
        quad(vertices + 6,
             vertex(x, y, z, wu, 1),
             vertex(-x, y, z, 0, 1),
             vertex(x, top, z, wu, 0),
             vertex(-x, top, z, 0, 0));
        quad(vertices + 12,
             vertex(-x, y, z, du, 1),
             vertex(-x, y, -z, 0, 1),
             vertex(-x, top, z, du, 0),
             vertex(-x, top, -z, 0, 0));
        quad(vertices + 18,
             vertex(-x, y, -z, wu, 1),
             vertex(x, y, -z, 0, 1),
             vertex(-x, top, -z, wu, 0),
             vertex(x, top, -z, 0, 0));
        building_material(materials[3], node.local_to_world, node.params);
        nglDxSetTexture(0, textures[3], 8, 3);
        draw_triangles(vertices, 8);
        const float ix = x - inset, iz = z - inset;

        const ProcVertex inner[] = {
            vertex(ix, y, iz, du, 0),   vertex(ix, y, -iz, 0, 0),    vertex(ix, top, -iz, 0, 1),
            vertex(ix, y, iz, du, 0),   vertex(ix, top, -iz, 0, 1),  vertex(ix, top, iz, du, 1),
            vertex(-ix, y, iz, wu, 0),  vertex(ix, y, iz, 0, 0),     vertex(ix, top, iz, 0, 1),
            vertex(-ix, y, iz, wu, 0),  vertex(ix, top, iz, 0, 1),   vertex(-ix, top, iz, wu, 1),
            vertex(-ix, y, -iz, du, 0), vertex(-ix, y, iz, 0, 0),    vertex(-ix, top, iz, 0, 1),
            vertex(-ix, y, -iz, du, 0), vertex(-ix, top, iz, 0, 1),  vertex(-ix, top, -iz, du, 1),
            vertex(ix, y, -iz, wu, 0),  vertex(-ix, y, -iz, 0, 0),   vertex(-ix, top, -iz, 0, 1),
            vertex(ix, y, -iz, wu, 0),  vertex(-ix, top, -iz, 0, 1), vertex(ix, top, -iz, wu, 1),
        };
        building_material(materials[2], node.local_to_world, node.params);
        nglDxSetTexture(0, textures[2], 8, 3);
        draw_triangles(inner, 8);
    }
    if (flags & 0x20) {
        const float hu = height * 6.0f;
        vertices[0] = vertex(x, -y, -z, 0, hu);
        vertices[1] = vertex(x, y, -z, 0, 0);
        vertices[2] = vertex(x - 1, -y, -z, 1, hu);
        vertices[3] = vertex(x - 1, y, -z, 1, 0);
        vertices[4] = vertex(x, -y, 1 - z, 1, hu);
        vertices[5] = vertex(x, y, 1 - z, 1, 0);
        vertices[6] = vertex(x, -y, z, 0, hu);
        vertices[7] = vertex(x, y, z, 0, 0);
        vertices[8] = vertex(x, -y, z - 1, 1, hu);
        vertices[9] = vertex(x, y, z - 1, 1, 0);
        vertices[10] = vertex(x - 1, -y, z, 1, hu);
        vertices[11] = vertex(x - 1, y, z, 1, 0);
        vertices[12] = vertex(-x, -y, z, 0, hu);
        vertices[13] = vertex(-x, y, z, 0, 0);
        vertices[14] = vertex(1 - x, -y, z, 1, hu);
        vertices[15] = vertex(1 - x, y, z, 1, 0);
        vertices[16] = vertex(-x, -y, z - 1, 1, hu);
        vertices[17] = vertex(-x, y, z - 1, 1, 0);
        vertices[18] = vertex(-x, -y, -z, 0, hu);
        vertices[19] = vertex(-x, y, -z, 0, 0);
        vertices[20] = vertex(-x, -y, 1 - z, 1, hu);
        vertices[21] = vertex(-x, y, 1 - z, 1, 0);
        vertices[22] = vertex(1 - x, -y, -z, 1, hu);
        vertices[23] = vertex(1 - x, y, -z, 1, 0);
        depth_bias(1);
        building_material(materials[1], node.local_to_world, node.params);
        nglDxSetTexture(0, textures[1], 8, 3);
        IDirect3DDevice9_DrawIndexedPrimitiveUP(g_Direct3DDevice,
                                                D3DPT_TRIANGLESTRIP,
                                                0,
                                                24,
                                                28,
                                                column_indices,
                                                D3DFMT_INDEX16,
                                                vertices,
                                                sizeof(ProcVertex));
        depth_bias(0);
    }
    if (record.floor_spacing) {
        const unsigned spacing = record.floor_spacing;
        const unsigned count = (record.height + spacing - 1) / spacing;
        static constexpr auto indices = [] {
            std::array<uint16_t, 240> table{};
            constexpr uint16_t band[] = {0, 4, 1, 5, 3, 7, 2, 6, 0, 4};
            for (unsigned i = 0; i < 24; ++i)
                for (unsigned j = 0; j < 10; ++j)
                    table[10 * i + j] = band[j];
            return table;
        }();
        const float du = depth * 6.0f, wu = width * 6.0f;
        for (unsigned i = 0; i < count; ++i) {
            const float top = y - i * spacing * 5.0f, bottom = top - 1.0f;
            auto *v = vertices + 8 * i;
            v[0] = vertex(x, top, -z, 0, 0);
            v[1] = vertex(x, top, z, du, 0);
            v[2] = vertex(-x, top, -z, wu, 0);
            v[3] = vertex(-x, top, z, wu + du, 0);
            v[4] = vertex(x, bottom, -z, 0, 1);
            v[5] = vertex(x, bottom, z, du, 1);
            v[6] = vertex(-x, bottom, -z, wu, 1);
            v[7] = vertex(-x, bottom, z, wu + du, 1);
        }
        depth_bias(2);
        building_material(materials[7], node.local_to_world, node.params);
        nglDxSetTexture(0, textures[7], 8, 3);
        IDirect3DDevice9_DrawIndexedPrimitiveUP(g_Direct3DDevice,
                                                D3DPT_TRIANGLESTRIP,
                                                0,
                                                8 * count,
                                                8 * count,
                                                indices.data(),
                                                D3DFMT_INDEX16,
                                                vertices,
                                                sizeof(ProcVertex));
        depth_bias(0);
    }
    if (EnableShader) {
        nglSetVertexDeclarationAndShader(&resources.roof);
        SetPixelShader(&resources.roof_pixel);
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &projected[0][0], 4);
        const float zero[4]{};
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 5, zero, 1);
    } else {
        nglSetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }
    if (flags & 0x10) {
        const float top = y + parapet_height, ix = x - inset, iz = z - inset;
        const float du = depth * 6.0f, wu = width * 6.0f, edge = inset * 0.2f;
        quad(vertices,
             vertex(x, top, -z, 0, 0),
             vertex(x, top, z, du, 0),
             vertex(ix, top, -iz, edge, 1),
             vertex(ix, top, iz, du - edge, 1));
        quad(vertices + 6,
             vertex(x, top, z, 0, 0),
             vertex(-x, top, z, wu, 0),
             vertex(ix, top, iz, edge, 1),
             vertex(-ix, top, iz, wu - edge, 1));
        quad(vertices + 12,
             vertex(-x, top, z, 0, 0),
             vertex(-x, top, -z, du, 0),
             vertex(-ix, top, iz, edge, 1),
             vertex(-ix, top, -iz, du - edge, 1));
        quad(vertices + 18,
             vertex(-x, top, -z, 0, 0),
             vertex(x, top, -z, wu, 0),
             vertex(-ix, top, -iz, edge, 1),
             vertex(ix, top, -iz, wu - edge, 1));
        if (!(flags & 0x40))
            depth_bias(1);
        material_colour(materials[4], node.params, 4);
        nglDxSetTexture(0, textures[4], 8, 3);
        draw_triangles(vertices, 8);
        if (!(flags & 0x40))
            depth_bias(0);
    }
    quad(vertices,
         vertex(x, y, -z, 0, 0),
         vertex(x, y, z, depth, 0),
         vertex(-x, y, -z, 0, width),
         vertex(-x, y, z, depth, width));
    material_colour(materials[5], node.params, 4);
    nglDxSetTexture(0, textures[5], 8, 3);
    draw_triangles(vertices, 2);
    g_renderState().setColourBufferWriteEnabled(15);
#else
    CDECL_CALL(0x00408A70, payload, nullptr);
#endif
}
