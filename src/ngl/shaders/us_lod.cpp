#include "us_lod.h"

#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "ngl_dx_shader.h"
#include "ngl_dx_texture.h"
#include "ngl_dx_state.h"
#include "us_shaderbase.h"
#include "fixedstring.h"
#include "variables.h"
#include "utility.h"
#include <cstring>
extern void sub_405CC0();

namespace {

constexpr DWORD lod_flat_vs[]{0xfffe0101, 0x1f,       0x80000000, 0x900f0000, 0x9, 0xc0010000, 0x90e40000, 0xa0e4000b,
                              0x9,        0xc0020000, 0x90e40000, 0xa0e4000c, 0x9, 0xc0040000, 0x90e40000, 0xa0e4000d,
                              0x9,        0xc0080000, 0x90e40000, 0xa0e4000e, 0x1, 0xd00f0000, 0xa0e40008, 0xffff};
constexpr DWORD lod_height_vs[]{0xfffe0101, 0x1f,       0x80000000, 0x900f0000, 0x9,        0xc0010000, 0x90e40000,
                                0xa0e4000b, 0x9,        0xc0020000, 0x90e40000, 0xa0e4000c, 0x9,        0xc0040000,
                                0x90e40000, 0xa0e4000d, 0x9,        0xc0080000, 0x90e40000, 0xa0e4000e, 0x9,
                                0x80020000, 0x90e40000, 0xa0e40001, 0x2,        0x80020002, 0x80550000, 0xa0550009,
                                0x5,        0x80020002, 0x80550002, 0xa0000009, 0x5,        0x80020002, 0x80550002,
                                0xa0aa0009, 0xa,        0x80020002, 0x80550002, 0xa0ff0009, 0xb,        0x80020002,
                                0x80550002, 0xa000005b, 0x1,        0xb0010000, 0x80550002, 0x1,        0x800f0003,
                                0xa0e42004, 0x5,        0xd00f0000, 0x80e40003, 0xa0e40008, 0xffff};
constexpr DWORD newlod_flat_vs[]{0xfffe0101, 0x1f,       0x80000000, 0x900f0000, 0x1f,       0x80000005, 0x900f0001,
                                 0x9,        0xc0010000, 0x90e40000, 0xa0e40000, 0x9,        0xc0020000, 0x90e40000,
                                 0xa0e40001, 0x9,        0xc0040000, 0x90e40000, 0xa0e40002, 0x9,        0xc0080000,
                                 0x90e40000, 0xa0e40003, 0x2,        0xe0030000, 0x90540001, 0xa0540004, 0xffff};
constexpr DWORD newlod_height_vs[]{
    0xfffe0101, 0x1f,       0x80000000, 0x900f0000, 0x1f,       0x80000005, 0x900f0001, 0x9,        0xc0010000,
    0x90e40000, 0xa0e40000, 0x9,        0xc0020000, 0x90e40000, 0xa0e40001, 0x9,        0xc0040000, 0x90e40000,
    0xa0e40002, 0x9,        0xc0080000, 0x90e40000, 0xa0e40003, 0x2,        0xe0030000, 0x90540001, 0xa0540004,
    0x2,        0x80020000, 0x90550000, 0xa0550005, 0x5,        0x80020000, 0x80550000, 0xa0000005, 0x5,
    0x80020000, 0x80550000, 0xa0aa0005, 0xa,        0x80020000, 0x80550000, 0xa0ff0005, 0xb,        0x80020000,
    0x80550000, 0xa000005b, 0x1,        0xb0010000, 0x80550000, 0x1,        0xd00f0000, 0xa0e42006, 0xffff};
constexpr D3DVERTEXELEMENT9 lod_elements[]{{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
                                           D3DDECL_END()};
constexpr D3DVERTEXELEMENT9 newlod_elements[]{
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()};
constexpr D3DVERTEXELEMENT9 fixed_lod_elements[]{
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    D3DDECL_END()};
Var<VShader[2]> lod_vertex{0x009706B0};
Var<IDirect3DPixelShader9 *[2]> lod_pixel{0x009706C0};
Var<VShader[2]> newlod_vertex{0x00970A00};
Var<IDirect3DPixelShader9 *[2]> newlod_pixel{0x00970A10};

void __fastcall lod_name(USLODShader *self, void *, tlFixedString *out)
{
    *out = self->_GetName();
}
void __fastcall newlod_name(NewlodShader *self, void *, tlFixedString *out)
{
    *out = self->_GetName();
}

void __fastcall empty_material(nglShader *, void *, nglMaterialBase *) {}
void __fastcall empty_sort(nglRenderNode *, void *, nglSortInfo *) {}
bool __fastcall switchable(nglShader *, void *)
{
    return false;
}

struct LODNode : nglShaderNode {
    nglMaterialBase *material;
    explicit LODNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
        : nglShaderNode(mesh, section), material(mat)
    {}
    void Render();
    void Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
    }
};
struct NewLODNode : LODNode {
    using LODNode::LODNode;
    void Render();
};

uint32_t packed_rgba(const float *rgba)
{
    return ((static_cast<uint32_t>(rgba[3] * 255.0f) & 255) << 24) |
           ((static_cast<uint32_t>(rgba[0] * 255.0f) & 255) << 16) |
           ((static_cast<uint32_t>(rgba[1] * 255.0f) & 255) << 8) | (static_cast<uint32_t>(rgba[2] * 255.0f) & 255);
}

void LODNode::Render()
{
    static Var<int> disabled{0x00956D58};
    static Var<uint32_t> initialized{0x00956D54};
    static Var<float> distance_squared{0x00956D50};
    static Var<nglMaterialBase *> top_material{0x00956358};
    if (disabled())
        return;
    if ((initialized() & 1) == 0) {
        initialized() |= 1;
        distance_squared() = 27889.0f;
    }
    const auto &center = m_meshNode->Mesh->SphereCenter;
    const auto &translation = m_meshNode->LocalToWorld[3];
    const auto &eye = nglCurScene->ViewPos;
    const float x = center[0] + translation[0] - eye[0];
    const float z = center[2] + translation[2] - eye[2];
    if (x * x + z * z < distance_squared() && eye[1] < 300.0f)
        return;
    const uint32_t variant = material != top_material();
    auto &state = g_renderState();
    if (variant == 0) {
        const auto *rgba = reinterpret_cast<const float *>(&material->field_1C) + 4 * g_TOD;
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 8, rgba, 1);
        } else {
            const auto packed = packed_rgba(rgba);
            if (state.field_9C != packed) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed);
                state.field_9C = packed;
            }
        }
    } else {
        nglSetupMaterialLighting(material, m_meshNode, 0, 8, 9, 4, 4);
    }
    if (state.m_cullingMode != D3DCULL_CW) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_CULLMODE, D3DCULL_CW);
        state.m_cullingMode = D3DCULL_CW;
    }
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 11, &m_meshNode->WorldToLocal[0][0], 4);
        nglSetVertexDeclarationAndShader(&lod_vertex()[variant]);
        state.setBlending(NGLBM_OPAQUE, 0, 0);
        SetPixelShader(&lod_pixel()[variant]);
    } else {
        static Var<uint32_t> generation{0x0091E1D8};
        if (m_meshSection->field_5C != generation()) {
            nglUpdateLODVertexColors(m_meshNode, m_meshSection);
            m_meshSection->field_5C = generation();
        }
        state.setBlending(NGLBM_OPAQUE, 0, 16);
        IDirect3DDevice9_SetTransform(g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(m_meshNode));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[0]);
        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
        nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
}

void NewLODNode::Render()
{
    static Var<int> disabled{0x00956D38};
    if (disabled())
        return;
    auto *payload = reinterpret_cast<char *>(material) - 4;
    auto &state = g_renderState();
    if (state.m_cullingMode != D3DCULL_CW) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_CULLMODE, D3DCULL_CW);
        state.m_cullingMode = D3DCULL_CW;
    }
    state.setBlending(NGLBM_OPAQUE, 0, 0);
    nglDxSetTexture(0, *reinterpret_cast<nglTexture **>(payload + 32), *reinterpret_cast<uint32_t *>(payload + 40), 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    if (EnableShader) {
        const float constants[]{0.0f, static_cast<float>(g_TOD), 0.0f, 255.00101f};
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, constants, 1);
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
        const uint32_t variant = *reinterpret_cast<uint8_t *>(payload + 116);
        if (variant != 0) {
            sub_405CC0();
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 5, nglHeightLightingConstants(), 1);
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 6, nglLightColors()[g_TOD], 4);
        }
        nglSetVertexDeclarationAndShader(&newlod_vertex()[variant]);
        SetPixelShader(&newlod_pixel()[variant]);
    }
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
}
}  // namespace

USLODShader::USLODShader()
{
    static void *table[]{func_address(&USLODShader::Register),
                         reinterpret_cast<void *>(lod_name),
                         func_address(&USLODShader::_AddNode),
                         func_address(&USLODShader::_BindMaterial),
                         reinterpret_cast<void *>(empty_material),
                         func_address(&USLODShader::_RebaseMaterial),
                         func_address(&nglShader::_CheckMaterialVersion),
                         func_address(&nglShader::_CheckVertexDefVersion),
                         func_address(&nglShader::_BindSection),
                         reinterpret_cast<void *>(switchable)};
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
#if STANDALONE_SYSTEM
    static Var<uint32_t> generation{0x0091E1D8};
    generation() = 1;
#endif
}
NewlodShader::NewlodShader()
{
    static void *table[]{func_address(&NewlodShader::Register),
                         reinterpret_cast<void *>(newlod_name),
                         func_address(&NewlodShader::_AddNode),
                         func_address(&NewlodShader::_BindMaterial),
                         func_address(&NewlodShader::_ReleaseMaterial),
                         func_address(&NewlodShader::_RebaseMaterial),
                         func_address(&nglShader::_CheckMaterialVersion),
                         func_address(&nglShader::_CheckVertexDefVersion),
                         func_address(&nglShader::_BindSection),
                         reinterpret_cast<void *>(switchable)};
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}
USLODShader &getUSLODShader()
{
    static Var<USLODShader> shader{0x0091E538};
    return shader();
}
NewlodShader &getNewlodShader()
{
    static Var<NewlodShader> shader{0x0091E544};
    return shader();
}
tlFixedString USLODShader::_GetName() const
{
    return tlFixedString{"USLOD"};
}
tlFixedString NewlodShader::_GetName() const
{
    return tlFixedString{"newlod"};
}
void USLODShader::Register()
{
    nglShader::_Register();
    if (EnableShader) {
        nglCreateVertexDeclarationAndShader(&lod_vertex()[0], lod_elements, lod_flat_vs);
        nglCreateVertexDeclarationAndShader(&lod_vertex()[1], lod_elements, lod_height_vs);
        auto debug_pixels_code = CompilePShader("shaders/debug_pixels.hlsl");
        CreatePixelShader(&lod_pixel()[0], debug_pixels_code.data());
        CreatePixelShader(&lod_pixel()[1], debug_pixels_code.data());
    } else if (dword_9738E0[0] == nullptr) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, fixed_lod_elements, &dword_9738E0[0]);
    }
}
void NewlodShader::Register()
{
    nglShader::_Register();
    if (EnableShader) {
        nglCreateVertexDeclarationAndShader(&newlod_vertex()[0], newlod_elements, newlod_flat_vs);
        nglCreateVertexDeclarationAndShader(&newlod_vertex()[1], newlod_elements, newlod_height_vs);
        auto texture_pixels_code = CompilePShader("shaders/texture_pixels.hlsl");
        CreatePixelShader(&newlod_pixel()[0], texture_pixels_code.data());
        auto particle_ps_code = CompilePShader("shaders/us_frontend_PS.hlsl");
        CreatePixelShader(&newlod_pixel()[1], particle_ps_code.data());
    }
}
void USLODShader::_AddNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
{
    auto *node = new (nglListAlloc(sizeof(LODNode), 16)) LODNode(mesh, section, material);
    static void *table[]{
        func_address(&LODNode::Render), reinterpret_cast<void *>(empty_sort), func_address(&LODNode::Delete)};
    node->m_vtbl = reinterpret_cast<decltype(node->m_vtbl)>(table);
    node->m_tex = reinterpret_cast<nglTexture *>(1);
    node->m_next_node = nglCurScene->OpaqueNodes;
    nglCurScene->OpaqueNodes = node;
    ++nglCurScene->OpaqueListCount;
}
void NewlodShader::_AddNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
{
    auto *node = new (nglListAlloc(sizeof(NewLODNode), 16)) NewLODNode(mesh, section, material);
    static void *table[]{
        func_address(&NewLODNode::Render), reinterpret_cast<void *>(empty_sort), func_address(&LODNode::Delete)};
    node->m_vtbl = reinterpret_cast<decltype(node->m_vtbl)>(table);
    node->m_tex = nullptr;
    node->m_next_node = nglCurScene->OpaqueNodes;
    nglCurScene->OpaqueNodes = node;
    ++nglCurScene->OpaqueListCount;
}
void USLODShader::_BindMaterial(nglMaterialBase *material)
{
    auto *colors = reinterpret_cast<float *>(&material->field_1C);
    for (uint32_t tod = 0; tod < 4; ++tod) {
        colors[4 * tod] *= 0.5f;
        colors[4 * tod + 1] *= 0.5f;
        colors[4 * tod + 2] *= 0.5f;
    }
}

void USLODShader::_RebaseMaterial(nglMaterialBase *, uint32_t) {}
void NewlodShader::_BindMaterial(nglMaterialBase *material)
{
    material->field_1C = nglLoadTexture(*material->field_18);
}
void NewlodShader::_ReleaseMaterial(nglMaterialBase *material)
{
    auto *texture = reinterpret_cast<nglTexture **>(reinterpret_cast<char *>(material) - 4 + 32);
    nglReleaseTexture(*texture);
    *texture = nullptr;
}
void NewlodShader::_RebaseMaterial(nglMaterialBase *material, uint32_t offset)
{
    auto **name = reinterpret_cast<tlFixedString **>(reinterpret_cast<char *>(material) - 4 + 28);
    if (*name != nullptr) {
        *name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(*name) + offset);
    }
}
void us_lod_patch()
{
    set_vfunc(0x00870D1C, func_address(&USLODShader::_BindMaterial));
    set_vfunc(0x00870D24, func_address(&USLODShader::_RebaseMaterial));
}
