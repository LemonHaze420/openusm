#include "us_simpleshader.h"

#include "ngl.h"
#include "trace.h"
#include "utility.h"
#include "us_interior.h"
#include "us_exterior.h"
#include "ngl_dx_shader.h"
#include "ngl_dx_state.h"
#include "ngl_dx_texture.h"
#include "ngl_scene.h"
#include "ngl_mesh.h"
#include "nglsortinfo.h"
#include "variables.h"
#include <cmath>
#include <functional>
#include <new>

#if STANDALONE_SYSTEM
namespace {

struct WorldMaterial {
    uint8_t header[0x1C];
    vector4d color[4];
    uint32_t animate_per_time_of_day;
    tlFixedString *texture_name;
    nglTexture *texture;
    uint32_t clamp_u, clamp_v, blend;
    float scroll_u, scroll_v;
    uint32_t cull, bias;
    float sort_bias;
};
static_assert(sizeof(WorldMaterial) == 0x88);
static_assert(offsetof(WorldMaterial, texture) == 0x64);


constexpr DWORD simple_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000002,
    0xE0030000, 0x90E40001, 0xA0540005, 0x00000005, 0xD00F0000, 0x90C00002, 0xA0E40004, 0x0000FFFF,
};
constexpr DWORD translucent_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000005,
    0xD00F0000, 0x90C00002, 0xA0E40004, 0x00000002, 0xE0030000, 0x90E40001, 0xA0540005, 0x0000FFFF,
};
constexpr DWORD world_pixel[] = {
    0xFFFF0101,
    0x00000042,
    0xB00F0000,
    0x00000005,
    0x800F0000,
    0xB0E40000,
    0x90E40000,
    0x0000FFFF,
};
constexpr D3DVERTEXELEMENT9 world_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    D3DDECL_END(),
};
template <bool Translucent>
struct WorldPrograms {
    VShader vertex{};
    IDirect3DPixelShader9 *pixel{};
    IDirect3DVertexDeclaration9 *fixed{};
    static WorldPrograms &get()
    {
        static WorldPrograms value;
        return value;
    }
};

template <bool Translucent, bool Trilinear>
struct WorldNode : nglShaderNode {
    nglMaterialBase *material;
    nglTexture *texture;

    WorldNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
        : nglShaderNode(mesh, section), material(mat)
    {
        const auto &data = *reinterpret_cast<WorldMaterial *>(mat);
        texture = data.texture;
        if ((texture->m_format & 0xFFu) == 16) {
            auto &params = mesh->field_8C;
            const uint32_t frame = params.IsSetParam<nglTextureFrameParam>()
                                       ? params.Get<nglTextureFrameParam>()->field_0
                                   : data.animate_per_time_of_day ? uint32_t(g_TOD) + 4u * nglCurScene->IFLFrame
                                                                  : nglCurScene->IFLFrame;
            texture = texture->Frames[frame % texture->m_num_palettes];
        }
        static void *table[]{
            func_address(&WorldNode::Render), func_address(&WorldNode::GetSortInfo), func_address(&WorldNode::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }


    void GetSortInfo(nglSortInfo &) {}
    WorldNode *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Render()
    {
        static Var<int> disabled{Translucent ? (Trilinear ? 0x956FC8 : 0x956FC4) : (Trilinear ? 0x956FB0 : 0x956FAC)};
        if (disabled())
            return;
        const auto &data = *reinterpret_cast<const WorldMaterial *>(material);
        auto &state = g_renderState();
        auto &programs = WorldPrograms<Translucent>::get();
        state.setCullingMode(data.cull == 2 ? D3DCULL_NONE : D3DCULL_CW);
        if (data.bias)
            nglSetDepthBias(Translucent ? 4.0f : float(data.bias + 1u));

        vector4d scroll{data.scroll_u * nglCurScene->AnimTime, data.scroll_v * nglCurScene->AnimTime, 0, 0};
        scroll.x -= std::floor(scroll.x);
        scroll.y -= std::floor(scroll.y);
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 5, &scroll.x, 1);
            nglSetVertexDeclarationAndShader(&programs.vertex);
        } else {
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, programs.fixed);
            if (std::not_equal_to<float>{}(data.scroll_u, 0.0f) || std::not_equal_to<float>{}(data.scroll_v, 0.0f)) {
                auto transform = identity_matrix;
                transform[2][0] = scroll.x;
                transform[2][1] = scroll.y;
                nglSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
                IDirect3DDevice9_SetTransform(
                    g_Direct3DDevice, D3DTS_TEXTURE0, reinterpret_cast<const D3DMATRIX *>(&transform));
            }
        }
        auto color = data.color[g_TOD];
        auto &params = m_meshNode->field_8C;
        if (params.IsSetParam<nglTintParam>()) {
            const auto &tint = *params.Get<nglTintParam>()->field_0;
            color = {color.x * tint.x, color.y * tint.y, color.z * tint.z, color.w * tint.w};
        }
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &color.x, 1);
        } else {
            const auto byte = [](float value) {
                return uint32_t(value * 255.0f) & 255u;
            };
            const uint32_t packed =
                byte(color.z) | (byte(color.y) << 8) | (byte(color.x) << 16) | (byte(color.w) << 24);
            if (state.field_9C != packed) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed);
                state.field_9C = packed;
            }
        }
        const auto blend = std::not_equal_to<float>{}(color.w, 1.0f) && data.blend <= 1
                               ? NGLBM_BLEND
                               : static_cast<nglBlendModeType>(data.blend);
        state.setBlending(blend, 0, 128);
        state.setColourBufferWriteEnabled(7);
        nglDxSetTexture(0, texture, 8, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, data.clamp_u ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, data.clamp_v ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
        if (EnableShader) {
            SetPixelShader(&programs.pixel);
        } else {
            nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
            nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
            nglSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            nglSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TFACTOR);
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, Translucent ? D3DTOP_MODULATE : D3DTOP_SELECTARG1);
            nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            nglSetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);
            nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        nglSetStreamSourceAndDrawPrimitive(m_meshSection);
        if (data.bias)
            nglSetDepthBias(0.0f);
        state.setColourBufferWriteEnabled(nglCurScene->FBWriteMask);
        if (!EnableShader)
            nglSetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }
};

template <bool Translucent, bool Trilinear>
struct WorldShader : nglShader {
    static void __fastcall Name(WorldShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{Translucent ? (Trilinear ? "USTranslucentTrilinear" : "SMTranslucent")
                                         : (Trilinear ? "USSimpleTrilinear" : "SMSimple")};
    }
    static bool __fastcall Switchable(WorldShader *, void *)
    {
        return true;
    }
    WorldShader()
    {
        static void *table[]{func_address(&WorldShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&WorldShader::Add),
                             func_address(&WorldShader::Bind),
                             func_address(&WorldShader::Release),
                             func_address(&WorldShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&WorldShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    WorldShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        auto &programs = WorldPrograms<Translucent>::get();
        if (EnableShader) {
            if (!programs.vertex.field_0) {
                nglCreateVertexDeclarationAndShader(
                    &programs.vertex, world_elements, Translucent ? translucent_vertex : simple_vertex);
                CreatePixelShader(&programs.pixel, world_pixel);
            }
        } else if (!programs.fixed) {
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, world_elements, &programs.fixed);
        }
    }
    void Bind(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<WorldMaterial *>(material);
        data.texture = nglLoadTexture(*data.texture_name);
        if constexpr (Translucent) {
            if (data.sort_bias > 0.0f)
                data.sort_bias = 0.0f;
        }
    }
    void Release(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<WorldMaterial *>(material);
        nglReleaseTexture(data.texture);
        data.texture = nullptr;
    }
    void Rebase(nglMaterialBase *material, unsigned int offset)
    {
        auto &name = reinterpret_cast<WorldMaterial *>(material)->texture_name;
        if (name)
            name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(name) + offset);
    }
    void Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
    {
        using Node = WorldNode<Translucent, Trilinear>;
        static_assert(sizeof(Node) == 0x1C);
        auto *node = new (nglListAlloc(sizeof(Node), 16)) Node{mesh, section, material};
        auto &params = mesh->field_8C;
        if (Translucent || (params.IsSetParam<nglTintParam>() &&
                            std::not_equal_to<float>{}(params.Get<nglTintParam>()->field_0->w, 1.0f))) {
            sub_417C10(node);
            if constexpr (Translucent) {
                const float distance =
                    bit_cast<float>(node->m_tex) + reinterpret_cast<WorldMaterial *>(material)->sort_bias;
                node->m_tex = bit_cast<nglTexture *>(distance);
            }
        } else {
            node->m_tex = reinterpret_cast<nglTexture *>(uint32_t(material->m_shader->field_8) << 24);
            node->m_next_node = nglCurScene->OpaqueNodes;
            nglCurScene->OpaqueNodes = node;
            ++nglCurScene->OpaqueListCount;
        }
    }
};
}  // namespace

void initialize_world_material_shaders()
{
    static WorldShader<false, false> simple;
    static WorldShader<false, true> simple_trilinear;
    static WorldShader<true, false> translucent;
    static WorldShader<true, true> translucent_trilinear;
}
#endif

namespace USSimpleShaderSpace {
template <>
void USSimpleShader<USInteriorMaterial>::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USSimpleShader<USInteriorMaterial>::BindMaterial");

#ifdef TARGET_XBOX
    a1->field_20 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_1C));
#else
    auto *Material = bit_cast<USInteriorMaterial *>(a1);
    Material->field_20 = nglLoadTexture(*Material->field_1C);
#endif
}

template <>
void USSimpleShader<USInteriorMaterial>::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("USSimpleShader<USInteriorMaterial>::RebaseMaterial");

    auto *v2 = a1->field_1C;
    if (v2 != nullptr) {
        a1->field_1C = (nglTexture *)((char *)v2 + a2);
    }
}

template <>
void USSimpleShader<USExteriorMaterial>::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USSimpleShader<USExteriorMaterial>::BindMaterial");

#ifdef TARGET_XBOX
    a1->field_20 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_1C));
#else
    auto *Material = bit_cast<USExteriorMaterial *>(a1);
    Material->field_64 = nglLoadTexture(*Material->field_60);
#endif
}

template <>
void USSimpleShader<USExteriorMaterial>::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("USSimpleShader<USExteriorMaterial>::RebaseMaterial");

    auto *Material = bit_cast<USExteriorMaterial *>(a1);
    auto *v2 = Material->field_60;
    if (v2 != nullptr) {
        Material->field_60 = CAST(Material->field_60, bit_cast<char *>(v2) + a2);
    }
}
}  // namespace USSimpleShaderSpace

void us_simpleshader_patch()
{
    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USInteriorMaterial>::_BindMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871658, address);
    }

    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USExteriorMaterial>::_BindMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871600, address);
    }

    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USInteriorMaterial>::_RebaseMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871660, address);
    }

    {
        auto func = &USSimpleShaderSpace::USSimpleShader<USExteriorMaterial>::_RebaseMaterial;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871608, address);
    }
}
