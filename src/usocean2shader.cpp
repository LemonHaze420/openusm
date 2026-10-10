#include "usocean2shader.h"

#include "func_wrapper.h"
#include "ngl.h"

#if STANDALONE_SYSTEM
#include <ngl_mesh.h>
#include <ngl_scene.h>
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include "utility.h"
#include "variables.h"
#include "nglshader.h"
#include "nglsortinfo.h"
#include <d3dx9tex.h>
#include <cmath>
#include <new>
namespace {
// @todo - shaders
constexpr DWORD ocean_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x00000001, 0x800F0000,
    0x90E40000, 0x00000001, 0x80020000, 0xA0AA0009, 0x00000009, 0xC0010000, 0x80E40000, 0xA0E40000, 0x00000009,
    0xC0020000, 0x80E40000, 0xA0E40001, 0x00000009, 0xC0040000, 0x80E40000, 0xA0E40002, 0x00000009, 0xC0080000,
    0x80E40000, 0xA0E40003, 0x00000002, 0xE0030000, 0x90540001, 0xA0540009, 0x00000002, 0x80030002, 0x80A80000,
    0xA154000A, 0x00000005, 0xE0030002, 0x80540002, 0xA0FE000A, 0x00000002, 0x800F0003, 0xA0E40004, 0x91E40000,
    0x00000008, 0x80080004, 0x80A40003, 0x80A40003, 0x00000007, 0x80080004, 0x80FF0004, 0x00000005, 0x80070003,
    0x80A40003, 0x80FF0004, 0x00000001, 0x80010005, 0xA0550004, 0x00000005, 0x80010005, 0x80000005, 0xA000000B,
    0x0000000B, 0x80010005, 0x80000005, 0xA000005B, 0x00000002, 0x80010005, 0x80000005, 0xA0AA005B, 0x00000007,
    0x80010006, 0x80000005, 0x00000001, 0x800F0007, 0xA0E40008, 0x00000002, 0x800F0008, 0xA0E40007, 0x81E40007,
    0x00000001, 0x80010009, 0x80550003, 0x0000000B, 0x800F0009, 0x80000009, 0x81000009, 0x00000002, 0x80010009,
    0xA0AA005B, 0x81000009, 0x00000005, 0x80010009, 0x80000009, 0x80000009, 0x00000005, 0x80010009, 0x80000009,
    0x80000009, 0x00000002, 0x8001000A, 0xA0AA005B, 0x81000009, 0x00000005, 0x8001000A, 0x8000000A, 0x80000006,
    0x00000004, 0xD00F0000, 0x80E40008, 0x8000000A, 0xA0E40008, 0x00000005, 0x80010009, 0x80000009, 0x80000009,
    0x00000005, 0x80010009, 0x80000009, 0x80000009, 0x00000005, 0x80010009, 0x80000009, 0x80000009, 0x00000002,
    0x80010009, 0xA0AA005B, 0x81000009, 0x00000005, 0xD0080000, 0xA0FF0008, 0x80000009, 0x00000002, 0x8008000B,
    0x80550003, 0x81550003, 0x00000001, 0x8007000B, 0x81580003, 0x00000008, 0x80010000, 0x80F4000B, 0x80F4000B,
    0x00000005, 0x80010000, 0x80000000, 0x80000000, 0x00000004, 0x80010000, 0x80000000, 0xA055005B, 0xA055005B,
    0x00000005, 0x8003000B, 0x8054000B, 0x80000000, 0x00000004, 0xE0030001, 0x8054000B, 0xA055005B, 0xA055005B,
    0x00000001, 0xD00F0001, 0xA0E40006, 0x00000001, 0xC00F0001, 0xA0AA005B, 0x0000FFFF,
};
// @todo - shaders
constexpr DWORD ocean_pixel[] = {
    0xFFFF0101, 0x00000042, 0xB00F0000, 0x00000042, 0xB00F0001, 0x00000042, 0xB00F0002, 0x00000005,
    0x800F0000, 0xB0FF0000, 0x90E40001, 0x00000005, 0x80070000, 0x80E40000, 0xB0E40001, 0x00000004,
    0x80170000, 0xB0E40000, 0x90E40000, 0x80E40000, 0x40000002, 0x80080000, 0x90FF0000, 0xB0FF0000,
    0x00000005, 0x80080000, 0x80FF0000, 0xB0FF0002, 0x0000FFFF,
};
struct OceanMaterial {
    uint8_t header[0x18];
    struct Texture {
        tlFixedString *name;
        nglTexture *texture;
    } textures[6];
    vector4d colour[4], highlight[4], reflection[4];
    float uv_scale, height;
};
static_assert(offsetof(OceanMaterial, uv_scale) == 0x108);
VShader ocean_program{};
IDirect3DPixelShader9 *ocean_pixel_program{};
IDirect3DVertexDeclaration9 *ocean_fixed{};
IDirect3DTexture9 *water_texture{};
int water_tod = -1;
constexpr D3DVERTEXELEMENT9 ocean_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END(),
};
constexpr D3DVERTEXELEMENT9 ocean_fixed_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END(),
};
void ocean_texture(unsigned stage, nglTexture *texture, unsigned flags, D3DTEXTUREADDRESS address)
{
    nglDxSetTexture(stage, texture, flags, 3);
    nglSetSamplerState(stage, D3DSAMP_ADDRESSU, address);
    nglSetSamplerState(stage, D3DSAMP_ADDRESSV, address);
}
struct OceanNode : nglShaderNode {
    nglMaterialBase *material;
    OceanNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *source)
        : nglShaderNode(mesh, section), material(source)
    {
        static void *table[]{
            func_address(&OceanNode::Render), func_address(&OceanNode::Sort), func_address(&OceanNode::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    void Sort(nglSortInfo &) {}
    OceanNode *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Render()
    {
        if (var<int>(0x956FD4))
            return;
        const auto &data = *reinterpret_cast<OceanMaterial *>(material);
        if (water_tod != g_TOD) {
            static const char *names[]{"Data\\packs\\water_day.dat",
                                       "Data\\packs\\water_night.dat",
                                       "Data\\packs\\water_rainy.dat",
                                       "Data\\packs\\water_sunset.dat"};
            if (water_texture)
                IDirect3DTexture9_Release(water_texture);
            water_texture = nullptr;
            D3DXCreateTextureFromFileA(g_Direct3DDevice, names[g_TOD], &water_texture);
            water_tod = g_TOD;
        }
        sub_413AF0();
        auto &state = g_renderState();
        state.setCullingMode(D3DCULL_NONE);
        state.setBlending(NGLBM_OPAQUE, 0, 0);
        state.setAlphaTesting(true);
        state.setAlphaFunction(D3DCMP_GREATER);
        state.setAlphaReferenceValue(0);
        state.setDepthBuffer(D3DZB_TRUE);
        state.setDepthBufferWriteEnabled(true);
        const auto &view = nglCurScene->ViewPos;
        if (EnableShader) {
            const auto inverse = m_meshNode->sub_4199D0();
            vector4d local_view{};
            for (unsigned j = 0; j < 4; ++j)
                local_view[j] =
                    view.x * inverse[0][j] + view.y * inverse[1][j] + view.z * inverse[2][j] + inverse[3][j];
            const float u = -data.uv_scale * view.x, v = data.uv_scale * view.z;
            const vector4d constants[]{local_view,
                                       {0, 1, 0, 0},
                                       data.reflection[g_TOD],
                                       data.highlight[g_TOD],
                                       data.colour[g_TOD],
                                       {u - std::floor(u), v - std::floor(v), data.height, 0},
                                       {1600.0f - view.x, -1900.0f - view.z, 1.0f / -3500.0f, 1.0f / 4400.0f},
                                       {0.025f, 0.25f, 2, 10}};
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &constants[0].x, 8);
            nglSetVertexDeclarationAndShader(&ocean_program);
            ocean_texture(0, data.textures[0].texture, 8, D3DTADDRESS_WRAP);
            ocean_texture(1, data.textures[1 + g_TOD].texture, 8, D3DTADDRESS_CLAMP);
            ocean_texture(2, data.textures[5].texture, 1, D3DTADDRESS_CLAMP);
            SetPixelShader(&ocean_pixel_program);
            nglSetStreamSourceAndDrawPrimitive(m_meshSection);
        } else {
            auto transform = identity_matrix;
            if (var<int>(0x956FD0)) {
                transform[3][0] = m_meshNode->LocalToWorld[3][0];
                transform[3][1] = data.height;
                transform[3][2] = m_meshNode->LocalToWorld[3][2];
            }
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&transform));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, ocean_fixed);
            ocean_texture(1, data.textures[5].texture, 1, D3DTADDRESS_CLAMP);
            const int alpha = view.y < 200.0f ? 255 : view.y <= 400.0f ? int((view.y - 200.0f) * 0.005f * 255.0f) : 0;
            constexpr uint32_t colours[]{0x677387, 0x0E1618, 0x2A233A, 0x36261D};
            if (alpha) {
                IDirect3DDevice9_SetTexture(
                    g_Direct3DDevice, 0, reinterpret_cast<IDirect3DBaseTexture9 *>(water_texture));
                g_renderTextureState().field_0[0] = water_texture;
                nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
            }
            if (alpha != 255)
                state.setBlendingFactor(colours[g_TOD] | (uint32_t(alpha) << 24));
            nglSetTextureStageState(
                0, D3DTSS_COLOROP, alpha && alpha != 255 ? D3DTOP_BLENDFACTORALPHA : D3DTOP_SELECTARG1);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, alpha == 255 ? D3DTA_TEXTURE : D3DTA_TFACTOR);
            if (alpha && alpha != 255)
                nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(0,
                                    alpha && alpha != 255 ? D3DTSS_ALPHAARG2 : D3DTSS_ALPHAARG1,
                                    alpha == 255 ? D3DTA_TEXTURE : D3DTA_TFACTOR);
            nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
            const float vertices[][7]{{-1900, -3, -1905, 0, 0, 0, 1},
                                      {-1900, -3, 2505, 0, 275.625f, 1, 1},
                                      {1600, -3, -1905, 218.75f, 0, 0, 0},
                                      {1600, -3, 2505, 218.75f, 275.625f, 0, 1}};
            IDirect3DDevice9_DrawPrimitiveUP(g_Direct3DDevice, D3DPT_TRIANGLESTRIP, 2, vertices, 28);
        }
    }
};
struct OceanShader : nglShader {
    static void __fastcall Name(OceanShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{"USOcean2"};
    }
    static bool __fastcall Switchable(OceanShader *, void *)
    {
        return true;
    }
    OceanShader()
    {
        static void *table[]{func_address(&OceanShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&OceanShader::Add),
                             func_address(&OceanShader::Bind),
                             func_address(&OceanShader::Release),
                             func_address(&OceanShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&OceanShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    OceanShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        if (EnableShader && !ocean_program.field_0) {
            nglCreateVertexDeclarationAndShader(&ocean_program, ocean_elements, ocean_vertex);
            CreatePixelShader(&ocean_pixel_program, ocean_pixel);
        } else if (!EnableShader && !ocean_fixed)
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, ocean_fixed_elements, &ocean_fixed);
    }
    void Bind(nglMaterialBase *material)
    {
        for (auto &texture : reinterpret_cast<OceanMaterial *>(material)->textures)
            texture.texture = nglLoadTexture(*texture.name);
    }
    void Release(nglMaterialBase *material)
    {
        for (auto &texture : reinterpret_cast<OceanMaterial *>(material)->textures) {
            nglReleaseTexture(texture.texture);
            texture.texture = nullptr;
        }
    }
    void Rebase(nglMaterialBase *material, unsigned offset)
    {
        for (auto &texture : reinterpret_cast<OceanMaterial *>(material)->textures)
            if (texture.name)
                texture.name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(texture.name) + offset);
    }
    void Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
    {
        if (mesh->Mesh != USOcean2Shader::OceanMesh())
            return;
        auto *node = new (nglListAlloc(sizeof(OceanNode), 16)) OceanNode{mesh, section, material};
        node->m_tex = reinterpret_cast<nglTexture *>(0xFF500000);
        node->m_next_node = nglCurScene->OpaqueNodes;
        nglCurScene->OpaqueNodes = node;
        ++nglCurScene->OpaqueListCount;
    }
};
}
void initialize_ocean_material_shader()
{
    static OceanShader shader;
}
#endif

Var<char *> USOcean2Shader::OceanMeshFileName{0x0091E1F4};

Var<nglMesh *> USOcean2Shader::OceanMesh{0x009562EC};

void USOcean2Shader::Init()
{
#if STANDALONE_SYSTEM
    const tlFixedString mesh_file_name{"oceanmesh"};
#else
    const tlFixedString mesh_file_name{USOcean2Shader::OceanMeshFileName()};
#endif
    USOcean2Shader::OceanMesh() = nglGetFirstMeshInFile(mesh_file_name);
}


void USOcean2Shader::Release()
{
    if (USOcean2Shader::OceanMesh() != nullptr) {
#if STANDALONE_SYSTEM
        const tlFixedString mesh_file_name{"oceanmesh"};
#else
        const tlFixedString mesh_file_name{USOcean2Shader::OceanMeshFileName()};
#endif
        nglReleaseMeshFile(mesh_file_name);
    }
}

void USOcean2Shader::Draw(const vector3d &a1)
{
#if STANDALONE_SYSTEM
    if (OceanMesh()) {
        auto transform = identity_matrix;
        transform[3][0] = a1.x;
        transform[3][2] = a1.z;
        nglListAddMesh(OceanMesh(), transform, nullptr, nullptr);
    }
#else
    CDECL_CALL(0x00408A00, &a1);
#endif
}
