#include "us_street.h"

#include "common.h"
#include "ngl.h"
#include "ngl_lighting.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "ngl_dx_shader.h"
#include "ngl_dx_state.h"
#include "ngl_dx_texture.h"
#include "utility.h"
#include "variables.h"

#include <cstddef>
#include <new>
#include <type_traits>
#include <functional>

namespace USStreetShaderSpace {
namespace {


struct ExteriorMaterial {
    char header[0x1C];
    vector4d Color[4];
    uint32_t AnimatePerTimeOfDay;
    tlFixedString *TextureName;
    nglTexture *Texture;
    uint32_t ClampU, ClampV, Blend, Cull, Projectors, Bias;
};
struct InteriorMaterial {
    char header[0x1C];
    tlFixedString *TextureName;
    nglTexture *Texture;
    uint32_t ClampU, ClampV, Blend, Cull, Projectors, Bias;
};
static_assert(offsetof(ExteriorMaterial, Texture) == 0x64);
static_assert(offsetof(ExteriorMaterial, Projectors) == 0x78);
static_assert(offsetof(InteriorMaterial, Texture) == 0x20);
static_assert(offsetof(InteriorMaterial, Projectors) == 0x34);

struct VertexShaders { VShader shader[6]; };
struct PixelShaders { IDirect3DPixelShader9 *shader[3]; };
Var<VertexShaders> vertex_shaders{0x0097092C};
Var<PixelShaders> pixel_shaders{0x00970920};
Var<int> suppress_exterior{0x00956FF4};
Var<int> suppress_interior{0x00956FF8};
Var<bool> suppress_street{0x00956354};
struct InteriorColor : vector4d {
    InteriorColor() : vector4d{1.0f, 1.0f, 1.0f, 1.0f} {}
};
Var<InteriorColor> interior_color{0x0091E138};



const D3DVERTEXELEMENT9 vertex_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    D3DDECL_END(),
};
constexpr DWORD vertex_0[] = {0xFFFE0101,0x0000001F,0x80000000,0x900F0000,0x0000001F,0x80000005,0x900F0001,0x0000001F,0x8000000A,0x900F0002,0x00000009,0xC0010000,0x90E40000,0xA0E40000,0x00000009,0xC0020000,0x90E40000,0xA0E40001,0x00000009,0xC0040000,0x90E40000,0xA0E40002,0x00000009,0xC0080000,0x90E40000,0xA0E40003,0x00000001,0xE0030000,0x90E40001,0x00000005,0xD00F0000,0x90C00002,0xA0E40004,0x0000FFFF};
constexpr DWORD vertex_1[] = {0xFFFE0101,0x0000001F,0x80000000,0x900F0000,0x0000001F,0x80000005,0x900F0001,0x0000001F,0x8000000A,0x900F0002,0x00000009,0xC0010000,0x90E40000,0xA0E40000,0x00000009,0xC0020000,0x90E40000,0xA0E40001,0x00000009,0xC0040000,0x90E40000,0xA0E40002,0x00000009,0xC0080000,0x90E40000,0xA0E40003,0x00000001,0xE0030000,0x90E40001,0x00000005,0xD00F0000,0x90C00002,0xA0E40004,0x00000009,0xE0010001,0x90E40000,0xA0E40005,0x00000009,0xE0020001,0x90E40000,0xA0E40006,0x0000FFFF};
constexpr DWORD vertex_2[] = {0xFFFE0101,0x0000001F,0x80000000,0x900F0000,0x0000001F,0x80000005,0x900F0001,0x0000001F,0x8000000A,0x900F0002,0x00000009,0xC0010000,0x90E40000,0xA0E40000,0x00000009,0xC0020000,0x90E40000,0xA0E40001,0x00000009,0xC0040000,0x90E40000,0xA0E40002,0x00000009,0xC0080000,0x90E40000,0xA0E40003,0x00000001,0xE0030000,0x90E40001,0x00000005,0xD00F0000,0x90C00002,0xA0E40004,0x00000009,0xE0010001,0x90E40000,0xA0E40005,0x00000009,0xE0020001,0x90E40000,0xA0E40006,0x00000009,0xE0010002,0x90E40000,0xA0E40008,0x00000009,0xE0020002,0x90E40000,0xA0E40009,0x0000FFFF};
constexpr DWORD vertex_3[] = {0xFFFE0101,0x0000001F,0x80000000,0x900F0000,0x0000001F,0x80000005,0x900F0001,0x0000001F,0x8000000A,0x900F0002,0x00000009,0xC0010000,0x90E40000,0xA0E40000,0x00000009,0xC0020000,0x90E40000,0xA0E40001,0x00000009,0xC0040000,0x90E40000,0xA0E40002,0x00000009,0xC0080000,0x90E40000,0xA0E40003,0x00000001,0xE0030000,0x90E40001,0x00000005,0xD00F0000,0x90E40002,0xA0E40004,0x0000FFFF};
constexpr DWORD vertex_4[] = {0xFFFE0101,0x0000001F,0x80000000,0x900F0000,0x0000001F,0x80000005,0x900F0001,0x0000001F,0x8000000A,0x900F0002,0x00000009,0xC0010000,0x90E40000,0xA0E40000,0x00000009,0xC0020000,0x90E40000,0xA0E40001,0x00000009,0xC0040000,0x90E40000,0xA0E40002,0x00000009,0xC0080000,0x90E40000,0xA0E40003,0x00000001,0xE0030000,0x90E40001,0x00000005,0xD00F0000,0x90E40002,0xA0E40004,0x00000009,0xE0010001,0x90E40000,0xA0E40005,0x00000009,0xE0020001,0x90E40000,0xA0E40006,0x0000FFFF};
constexpr DWORD vertex_5[] = {0xFFFE0101,0x0000001F,0x80000000,0x900F0000,0x0000001F,0x80000005,0x900F0001,0x0000001F,0x8000000A,0x900F0002,0x00000009,0xC0010000,0x90E40000,0xA0E40000,0x00000009,0xC0020000,0x90E40000,0xA0E40001,0x00000009,0xC0040000,0x90E40000,0xA0E40002,0x00000009,0xC0080000,0x90E40000,0xA0E40003,0x00000001,0xE0030000,0x90E40001,0x00000005,0xD00F0000,0x90E40002,0xA0E40004,0x00000009,0xE0010001,0x90E40000,0xA0E40005,0x00000009,0xE0020001,0x90E40000,0xA0E40006,0x00000009,0xE0010002,0x90E40000,0xA0E40008,0x00000009,0xE0020002,0x90E40000,0xA0E40009,0x0000FFFF};
constexpr DWORD pixel_0[] = {0xFFFF0101,0x00000042,0xB00F0000,0x00000005,0x80070000,0xB0E40000,0x90E40000,0x40000005,0x80080000,0xB0FF0000,0x90FF0000,0x0000FFFF};
constexpr DWORD pixel_1[] = {0xFFFF0101,0x00000042,0xB00F0000,0x00000042,0xB00F0001,0x00000005,0x80070000,0xB0E40000,0x90E40000,0x00000005,0x80070000,0x80E40000,0xB6FF0001,0x40000005,0x80080000,0xB0FF0000,0x90FF0000,0x0000FFFF};
constexpr DWORD pixel_2[] = {0xFFFF0101,0x00000042,0xB00F0000,0x00000042,0xB00F0001,0x00000042,0xB00F0002,0x00000005,0x80070000,0xB0E40000,0x90E40000,0x00000005,0x80070000,0x80E40000,0xB6FF0001,0x00000005,0x80070000,0x80E40000,0xB6FF0002,0x40000005,0x80080000,0xB0FF0000,0x90FF0000,0x0000FFFF};

vector4d transformBasis(const vector4d &v, const matrix4x4 &m)
{
    return {v.x * m.arr[0].x + v.y * m.arr[1].x + v.z * m.arr[2].x,
            v.x * m.arr[0].y + v.y * m.arr[1].y + v.z * m.arr[2].y,
            v.x * m.arr[0].z + v.y * m.arr[1].z + v.z * m.arr[2].z, 0.0f};
}


nglTexture *selectTexture(nglTexture *tex, nglMeshNode *mesh, bool perTimeOfDay)
{
    if ((tex->m_format & 0xFFu) != 16)
        return tex;
    auto &params = mesh->field_8C;
    const uint32_t frame = params.IsSetParam<nglTextureFrameParam>()
        ? params.Get<nglTextureFrameParam>()->field_0
        : perTimeOfDay ? uint32_t(g_TOD) + 4u * nglCurScene->IFLFrame : nglCurScene->IFLFrame;
    return tex->Frames[frame % tex->m_num_palettes];
}

template <bool Interior>
void __fastcall shaderName(USGroundShader<Interior> *shader, void *, tlFixedString *out)
{
    *out = shader->_GetName();
}


void __fastcall bindSection(nglShader *, void *, nglMeshSection *) {}
bool __fastcall materialVersion(nglShader *, void *, nglMaterialBase *) { return true; }
bool __fastcall vertexVersion(nglShader *, void *, nglMeshSection *) { return true; }
bool __fastcall isSwitchable(nglShader *, void *) { return true; }
}

VALIDATE_SIZE(USGroundShader<false>, 0xC);
VALIDATE_SIZE(USGroundShader<true>, 0xC);
VALIDATE_SIZE(USStreetShaderNode<false>, 0x1C);
VALIDATE_SIZE(USStreetShaderNode<true>, 0x1C);

template <bool Interior>
USGroundShader<Interior>::USGroundShader()
{
    static void *table[] = {
        func_address(&USGroundShader::Register), reinterpret_cast<void *>(shaderName<Interior>),
        func_address(&USGroundShader::_AddNode), func_address(&USGroundShader::_BindMaterial),
        func_address(&USGroundShader::_ReleaseMaterial), func_address(&USGroundShader::_RebaseMaterial),
        reinterpret_cast<void *>(materialVersion), reinterpret_cast<void *>(vertexVersion),
        reinterpret_cast<void *>(bindSection), reinterpret_cast<void *>(isSwitchable),
        func_address(&USGroundShader::Delete),
    };
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}

template <bool Interior>
USGroundShader<Interior> *USGroundShader<Interior>::Delete(unsigned char flags)
{
    if (flags & 1)
        ::operator delete(this);
    return this;
}

USGroundShader<false> &getUSStreetShader()
{
    static Var<USGroundShader<false>> shader{0x0091E4E4};
    return shader();
}
USGroundShader<true> &getUSFloorShader()
{
    static Var<USGroundShader<true>> shader{0x0091E4F0};
    return shader();
}

template <bool Interior>
tlFixedString USGroundShader<Interior>::_GetName() const
{
    return tlFixedString{Interior ? "USFloor" : "USStreet"};
}

template <bool Interior>
void USGroundShader<Interior>::Register()
{
    nglShader::_Register();
    if (EnableShader) {
        const DWORD *vertex_programs[] = {vertex_0, vertex_1, vertex_2, vertex_3, vertex_4, vertex_5};
        const DWORD *pixel_programs[] = {pixel_0, pixel_1, pixel_2};
        for (int i = 0; i < 6; ++i)
            nglCreateVertexDeclarationAndShader(&vertex_shaders().shader[i], vertex_elements, vertex_programs[i]);
        for (int i = 0; i < 3; ++i)
            CreatePixelShader(&pixel_shaders().shader[i], pixel_programs[i]);
    } else if (!dword_9738E0[3]) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, vertex_elements, &dword_9738E0[3]);
    }
}

template <bool Interior>
void USGroundShader<Interior>::_BindMaterial(nglMaterialBase *material)
{
    using Material = std::conditional_t<Interior, InteriorMaterial, ExteriorMaterial>;
    auto *mat = reinterpret_cast<Material *>(material);
    mat->Texture = nglLoadTexture(*mat->TextureName);
}

template <bool Interior>
void USGroundShader<Interior>::_ReleaseMaterial(nglMaterialBase *material)
{
    using Material = std::conditional_t<Interior, InteriorMaterial, ExteriorMaterial>;
    auto *mat = reinterpret_cast<Material *>(material);
    nglReleaseTexture(mat->Texture);
    mat->Texture = nullptr;
}

template <bool Interior>
void USGroundShader<Interior>::_RebaseMaterial(nglMaterialBase *material, unsigned int offset)
{
    using Material = std::conditional_t<Interior, InteriorMaterial, ExteriorMaterial>;
    auto *mat = reinterpret_cast<Material *>(material);
    if (mat->TextureName)
        mat->TextureName = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(mat->TextureName) + offset);
}

template <bool Interior>
USStreetShaderNode<Interior>::USStreetShaderNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
    : nglShaderNode(mesh, section), material(mat)
{
    using Material = std::conditional_t<Interior, InteriorMaterial, ExteriorMaterial>;
    auto *payload = reinterpret_cast<Material *>(mat);
    bool perTimeOfDay = false;
    if constexpr (!Interior)
        perTimeOfDay = payload->AnimatePerTimeOfDay != 0;
    texture = selectTexture(payload->Texture, mesh, perTimeOfDay);
    static void *table[] = {func_address(&USStreetShaderNode::Render),
        func_address(&USStreetShaderNode::GetSortInfo), func_address(&USStreetShaderNode::Delete)};
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}

template <bool Interior>
void USStreetShaderNode<Interior>::GetSortInfo(nglSortInfo &)
{

}

template <bool Interior>
USStreetShaderNode<Interior> *USStreetShaderNode<Interior>::Delete(unsigned char flags)
{
    if (flags & 1)
        ::operator delete(this);
    return this;
}

template <bool Interior>
void USGroundShader<Interior>::_AddNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
{
    auto *storage = nglListAlloc(sizeof(USStreetShaderNode<Interior>), 16);
    auto *node = new (storage) USStreetShaderNode<Interior>{mesh, section, mat};
    auto &params = mesh->field_8C;
    if (params.IsSetParam<nglTintParam>() &&
        std::not_equal_to<float>{}(params.Get<nglTintParam>()->field_0->w, 1.0f)) {
        sub_417C10(node);
    } else {
        node->m_tex = reinterpret_cast<nglTexture *>(uint32_t(mat->m_shader->field_8) << 24);
        node->m_next_node = nglCurScene->OpaqueNodes;
        nglCurScene->OpaqueNodes = node;
        ++nglCurScene->OpaqueListCount;
    }
}

template <bool Interior>
void USStreetShaderNode<Interior>::Render()
{
    if ((Interior ? suppress_interior() : suppress_exterior()) || suppress_street())
        return;
    sub_413AF0();
    using Material = std::conditional_t<Interior, InteriorMaterial, ExteriorMaterial>;
    const auto &mat = *reinterpret_cast<Material *>(material);
    auto &state = g_renderState();
    state.setCullingMode(mat.Cull == 0 ? D3DCULL_CW : D3DCULL_NONE);
    const bool floor = material->m_shader == &getUSFloorShader();
    uint32_t stage = 0;
    if (mat.Projectors) {
        m_meshNode->Mesh->Flags |= 0x02000000u;
        nglDetermineProjLights(m_meshNode);
        auto *head = &nglCurLightContext()->ProjectorHead;
        for (auto *node = head->SelectedNext; node != head && stage < 2; node = node->SelectedNext) {
            if (node->Type != NGL_LIGHT_DIR_PROJECTOR)
                continue;
            const auto &light = *static_cast<nglDirProjectorLightInfo *>(node->Data);


            const auto &local = m_meshNode->LocalToWorld;
            const auto &uv = light.WorldToUV;
            const vector4d x = transformBasis(local.arr[0], uv);
            const vector4d y = transformBasis(local.arr[1], uv);
            const vector4d z = transformBasis(local.arr[2], uv);
            vector4d translation = transformBasis(local.w, uv);
            translation.x += uv.w.x;
            translation.y += uv.w.y;
            const vector4d direction = transformBasis(light.ZAxis, m_meshNode->sub_4199D0());
            const vector4d constants[] = {
                {x.x, y.x, z.x, translation.x},
                {x.y, y.y, z.y, translation.y},
                {direction.x, direction.y, direction.z, 1.0f},
            };
            ++stage;
            nglDxSetTexture(stage, light.Texture, 4, 3);
            nglSetSamplerState(stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            nglSetSamplerState(stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            if (EnableShader)
                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 2 + 3 * stage, &constants[0].x, 3);
        }
    }
    if (mat.Bias)
        nglSetDepthBias(1.0f);
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal.arr[0].x, 4);
        nglSetVertexDeclarationAndShader(&vertex_shaders().shader[stage + 3 * floor]);
    } else {
        IDirect3DDevice9_SetTransform(g_Direct3DDevice, D3DTS_WORLD,
            reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[3]);
    }
    vector4d color;
    if constexpr (Interior)
        color = interior_color();
    else
        color = mat.Color[g_TOD];
    auto &params = m_meshNode->field_8C;
    if (params.IsSetParam<nglTintParam>()) {
        const auto &tint = *params.Get<nglTintParam>()->field_0;
        color = {color.x * tint.x, color.y * tint.y, color.z * tint.z, color.w * tint.w};
    }
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &color.x, 1);
    } else {
        const auto toByte = [](float v) { return uint32_t(v * 255.0f) & 0xFFu; };
        const uint32_t packed = toByte(color.z) | (toByte(color.y) << 8) |
                                (toByte(color.x) << 16) | (toByte(color.w) << 24);
        if (state.field_9C != packed) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed);
            state.field_9C = packed;
        }
    }
    const auto blend = std::not_equal_to<float>{}(color.w, 1.0f) && mat.Blend <= 1
        ? NGLBM_BLEND : static_cast<nglBlendModeType>(mat.Blend);
    state.setBlending(blend, 0, 128);
    state.setColourBufferWriteEnabled(7);
    nglDxSetTexture(0, texture, 8, 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, mat.ClampU ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, mat.ClampV ? D3DTADDRESS_CLAMP : D3DTADDRESS_WRAP);
    if (EnableShader) {
        SetPixelShader(&pixel_shaders().shader[stage]);
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
        nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
        nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
        nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    }
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    if (stage)
        nglResetTextureState();
    if (mat.Bias)
        nglSetDepthBias(0.0f);
}

template struct USGroundShader<false>;
template struct USGroundShader<true>;
template struct USStreetShaderNode<false>;
template struct USStreetShaderNode<true>;

}

