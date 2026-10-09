#include "ngldebugshader.h"

#include "func_wrapper.h"
#include "ngl.h"
#include "nglsortinfo.h"
#include "variables.h"
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include <ngl_mesh.h>
#include <ngl_scene.h>
#include <new>

#if STANDALONE_SYSTEM
namespace {
Var<VShader> debug_program{0x00976E74};
Var<IDirect3DPixelShader9 *> debug_pixel{0x00976E70};
constexpr D3DVERTEXELEMENT9 debug_elements[]{
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0}, D3DDECL_END()};
constexpr DWORD debug_vertex[]{0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x00000009, 0xC0010000,
                               0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
                               0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000,
                               0x90E40000, 0xA0E40003, 0x00000001, 0xD00F0000, 0xA0E40004, 0x00000001,
                               0xD00F0001, 0xA000005B, 0x00000001, 0xC00F0001, 0xA0AA005B, 0x0000FFFF};
constexpr DWORD debug_pixels[]{0xFFFF0101, 0x00000001, 0x800F0000, 0x90E40000, 0x0000FFFF};
struct DebugNode : nglShaderNode {
    nglMaterialBase *material;
    DebugNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *source)
        : nglShaderNode(mesh, section), material(source)
    {
        static void *table[]{
            func_address(&DebugNode::Render), func_address(&DebugNode::Sort), func_address(&DebugNode::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    DebugNode *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Sort(nglSortInfo &out)
    {
        out.Type = NGLSORT_TRANSLUCENT;
        const auto world = sub_414360(m_meshSection->SphereCenter, m_meshNode->LocalToWorld);
        const auto view = sub_414360(world, nglCurScene->WorldToView);
        out.Dist = view[2] + m_meshSection->SphereRadius;
    }
    void Render()
    {
        auto &state = g_renderState();
        state.setCullingMode(D3DCULL_NONE);
        state.setBlending(NGLBM_BLEND, 0, 0);
        const vector4d white{1, 1, 1, 1};
        auto &params = m_meshNode->field_8C;
        const auto &tint = params.IsSetParam<nglTintParam>() ? *params.Get<nglTintParam>()->field_0 : white;
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &tint.x, 1);
            nglSetVertexDeclarationAndShader(&debug_program());
            SetPixelShader(&debug_pixel());
        } else {
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[23]);
            const auto byte = [](float value) {
                return static_cast<uint32_t>(value * 255.0f) & 0xFFu;
            };
            const uint32_t colour = (static_cast<uint32_t>(tint.w * 255.0f) << 24) | (byte(tint.x) << 16) |
                                    (byte(tint.y) << 8) | byte(tint.z);
            state.setBlendingFactor(colour);
            nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
            nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    }
};
static_assert(sizeof(DebugNode) == 24);
void __fastcall debug_name(nglDebugShader *, void *, tlFixedString *out)
{
    *out = tlFixedString{"Debug"};
}
void __fastcall debug_add(nglDebugShader *, void *, nglMeshNode *mesh, nglMeshSection *section,
                          nglMaterialBase *material)
{
    auto *node = new (nglListAlloc(sizeof(DebugNode), 16)) DebugNode{mesh, section, material};
    nglListAddNode(node);
}
void __fastcall debug_material(nglDebugShader *, void *, nglMaterialBase *) {}
void __fastcall debug_rebase(nglDebugShader *, void *, nglMaterialBase *, unsigned) {}
bool __fastcall debug_switchable(nglDebugShader *, void *)
{
    return false;
}
nglDebugShader *__fastcall debug_delete(nglDebugShader *self, void *, unsigned char flags)
{
    if (flags & 1)
        ::operator delete(self);
    return self;
}
}
#endif

nglDebugShader::nglDebugShader()
{
#if STANDALONE_SYSTEM
    static void *table[]{func_address(&nglDebugShader::Register),
                         reinterpret_cast<void *>(debug_name),
                         reinterpret_cast<void *>(debug_add),
                         reinterpret_cast<void *>(debug_material),
                         reinterpret_cast<void *>(debug_material),
                         reinterpret_cast<void *>(debug_rebase),
                         func_address(&nglShader::_CheckMaterialVersion),
                         func_address(&nglShader::_CheckVertexDefVersion),
                         func_address(&nglShader::_BindSection),
                         reinterpret_cast<void *>(debug_switchable),
                         reinterpret_cast<void *>(debug_delete)};
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
#endif
}

int nglDebugShader::Register()
{
#if STANDALONE_SYSTEM
    nglShader::_Register();
    if (EnableShader) {
        nglCreateVertexDeclarationAndShader(&debug_program(), debug_elements, debug_vertex);
        CreatePixelShader(&debug_pixel(), debug_pixels);
    } else if (!dword_9738E0[23]) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, debug_elements, &dword_9738E0[23]);
    }
    return 0;
#else
    return THISCALL(0x00783790, this);
#endif
}

#if STANDALONE_SYSTEM
void initialize_debug_material_shader()
{
    static nglDebugShader shader;
}
#endif
