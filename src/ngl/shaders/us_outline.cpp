#include "us_outline.h"

#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>

#include "us_exterior.h"
#include "us_interior.h"

#include "func_wrapper.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "trace.h"
#include "utility.h"
#include "variable.h"
#include "variables.h"

#include "ngl_lighting.h"
#include <d3dx9math.h>
#include <cstddef>
#include <functional>
#include <new>

#if STANDALONE_SYSTEM
extern void sub_413850(nglMaterialBase *, nglParamSet<nglShaderParamSet_Pool> *, color *);
extern nglDirLightInfo *nglGetLightAsDirLight(nglDirLightInfo *, nglLightNode *, math::VecClass<3, 1>);

namespace {
void initialize_outline_material_shaders();

struct ShinyMaterial {
    uint8_t header[0x1C];
    vector4d color[4];
    uint32_t animate_per_time_of_day;
    tlFixedString *texture_name;
    nglTexture *texture;
    uint32_t blend;
    tlFixedString *shine_name;
    nglTexture *shine;
    uint32_t cull;
    bool tod_frames() const
    {
        return animate_per_time_of_day != 0;
    }
};
static_assert(sizeof(ShinyMaterial) == 0x78);
static_assert(offsetof(ShinyMaterial, texture_name) == 0x60);
static_assert(offsetof(ShinyMaterial, texture) == 0x64);
static_assert(offsetof(ShinyMaterial, blend) == 0x68);
static_assert(offsetof(ShinyMaterial, shine_name) == 0x6C);
static_assert(offsetof(ShinyMaterial, shine) == 0x70);
static_assert(offsetof(ShinyMaterial, cull) == 0x74);

struct ShinyInteriorMaterial {
    uint8_t header[0x1C];
    tlFixedString *texture_name;
    nglTexture *texture;
    uint32_t blend;
    tlFixedString *shine_name;
    nglTexture *shine;
    uint32_t cull;
    bool tod_frames() const
    {
        return false;
    }
};
static_assert(offsetof(ShinyInteriorMaterial, cull) == 0x30);
template <bool Interior>
using ShinyPayload = std::conditional_t<Interior, ShinyInteriorMaterial, ShinyMaterial>;


constexpr D3DVERTEXELEMENT9 shiny_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    {0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BINORMAL, 0},
    {0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0},
    {0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 56, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    D3DDECL_END(),
};
constexpr D3DVERTEXELEMENT9 shiny_fixed_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    {0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
    {0, 36, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
    {0, 48, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 56, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    D3DDECL_END(),
};
constexpr D3DVERTEXELEMENT9 shiny_chrome_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
    {0, 40, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    {0, 44, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1},
    D3DDECL_END(),
};
constexpr DWORD shiny_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000003, 0x900F0001, 0x0000001F, 0x80000007,
    0x900F0002, 0x0000001F, 0x80000006, 0x900F0003, 0x0000001F, 0x80000005, 0x900F0004, 0x0000001F, 0x8000000A,
    0x900F0005, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000001,
    0xE0030000, 0x90E40004, 0x00000005, 0xD00F0000, 0x90C00005, 0xA0E40004, 0x00000008, 0x800F0000, 0x90E40001,
    0xA1E40005, 0x0000000D, 0xD00F0001, 0x80E40000, 0xA000005B, 0x00000002, 0x800F0002, 0xA0E40006, 0x91E40000,
    0x00000008, 0x80080003, 0x80A40002, 0x80A40002, 0x00000007, 0x80080003, 0x80FF0003, 0x00000005, 0x80070002,
    0x80A40002, 0x80FF0003, 0x00000002, 0x800F0004, 0x80E40002, 0xA1E40005, 0x00000008, 0x80080005, 0x80A40004,
    0x80A40004, 0x00000007, 0x80080005, 0x80FF0005, 0x00000005, 0x80070004, 0x80A40004, 0x80FF0005, 0x00000008,
    0x80010000, 0x80E40004, 0x90E40003, 0x00000008, 0x80020000, 0x80E40004, 0x90E40002, 0x00000004, 0xE0030001,
    0x80E40000, 0xA055005B, 0xA055005B, 0x0000FFFF,
};
constexpr DWORD shiny_vertex_linear[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000003, 0x900F0001, 0x0000001F, 0x80000007,
    0x900F0002, 0x0000001F, 0x80000006, 0x900F0003, 0x0000001F, 0x80000005, 0x900F0004, 0x0000001F, 0x8000000A,
    0x900F0005, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000001,
    0xE0030000, 0x90E40004, 0x00000005, 0xD00F0000, 0x90E40005, 0xA0E40004, 0x00000008, 0x800F0000, 0x90E40001,
    0xA1E40005, 0x0000000D, 0xD00F0001, 0x80E40000, 0xA000005B, 0x00000002, 0x800F0002, 0xA0E40006, 0x91E40000,
    0x00000008, 0x80080003, 0x80A40002, 0x80A40002, 0x00000007, 0x80080003, 0x80FF0003, 0x00000005, 0x80070002,
    0x80A40002, 0x80FF0003, 0x00000002, 0x800F0004, 0x80E40002, 0xA1E40005, 0x00000008, 0x80080005, 0x80A40004,
    0x80A40004, 0x00000007, 0x80080005, 0x80FF0005, 0x00000005, 0x80070004, 0x80A40004, 0x80FF0005, 0x00000008,
    0x80010000, 0x80E40004, 0x90E40003, 0x00000008, 0x80020000, 0x80E40004, 0x90E40002, 0x00000004, 0xE0030001,
    0x80E40000, 0xA055005B, 0xA055005B, 0x0000FFFF,
};
constexpr DWORD shiny_pixel[] = {
    0xFFFF0101, 0x00000042, 0xB00F0000, 0x00000042, 0xB00F0001, 0x00000005, 0x800F0001, 0xB0E40001,
    0xB0FF0000, 0x00000005, 0x800F0001, 0x80E40001, 0x90FF0001, 0x00000004, 0x80070000, 0xB0E40000,
    0x90E40000, 0x80E40001, 0x40000001, 0x80080000, 0x90FF0000, 0x0000FFFF,
};
struct ShinyPrograms {
    VShader vertex[2];
};
Var<ShinyPrograms> shiny_programs{0x00970864};
Var<IDirect3DPixelShader9 *> shiny_pixel_shader{0x00970860};
Var<int> suppress_shiny{0x00956FEC};

vector4d shinyBasis(const vector4d &v, const matrix4x4 &m)
{
    return {v.x * m.arr[0].x + v.y * m.arr[1].x + v.z * m.arr[2].x,
            v.x * m.arr[0].y + v.y * m.arr[1].y + v.z * m.arr[2].y,
            v.x * m.arr[0].z + v.y * m.arr[1].z + v.z * m.arr[2].z,
            0.0f};
}


vector4d shinyLightDirection(nglMeshNode *mesh)
{
    auto *context = mesh->field_8C.IsSetParam<nglLightContextParam>()
                        ? mesh->field_8C.Get<nglLightContextParam>()->field_0
                        : nglCurScene->field_350;
    nglCurLightContext() = context;
    auto *head = &context->Head;
    head->SelectedNext = head;
    const uint32_t category = mesh->Mesh->Flags >> NGL_LIGHTCAT_SHIFT;
    if (category) {
        const auto center = sub_414360(mesh->Mesh->SphereCenter, mesh->LocalToWorld);
        for (auto *light = head->Next[category - 1]; light != head; light = light->Next[category - 1]) {
            bool selected = light->Type == NGL_LIGHT_DIRECTIONAL;
            if (light->Type == NGL_LIGHT_POINT) {
                const auto &point = *static_cast<nglPointLightInfo *>(light->Data);
                const float x = point.ViewPos.x - center.x;
                const float y = point.ViewPos.y - center.y;
                const float z = point.ViewPos.z - center.z;
                const float radius = point.Far + mesh->Mesh->SphereRadius;
                selected = radius * radius >= z * z + y * y + x * x;
            }
            if (selected) {
                light->SelectedNext = head->SelectedNext;
                head->SelectedNext = light;
            }
        }
    }
    if (head->SelectedNext == head)
        return {0.0f, 1.0f, 0.0f, 0.0f};
    nglDirLightInfo scratch;
    const auto *light = nglGetLightAsDirLight(&scratch, head->SelectedNext, mesh->LocalToWorld.w);
    return {light->Dir.x, light->Dir.y, light->Dir.z, light->Dir.w};
}


void shinyChromeVertices(nglMeshSection *section, const vector4d &light, const vector4d &view)
{
    struct Vertex {
        float position[3], normal[3], binormal[3], tangent[3], uv[2];
        uint32_t color;
    };
    struct ChromeVertex {
        float position[3], normal[3], uv[2], shine_uv[2];
        uint32_t color, light_color;
    };
    static_assert(sizeof(Vertex) == 60);
    static_assert(sizeof(ChromeVertex) == 48);
    auto *source = reinterpret_cast<const Vertex *>(section->field_3C.getVertexData());
    ChromeVertex *output;
    auto *buffer = section->field_3C.getVertexBuffer();
    IDirect3DVertexBuffer9_Lock(buffer, 0, 0, reinterpret_cast<void **>(&output), D3DLOCK_DISCARD);
    for (uint32_t i = 0; i < section->field_3C.getSize() / 60u; ++i) {
        const auto &vertex = source[i];
        auto &converted = output[i];
        for (int lane = 0; lane < 3; ++lane) {
            converted.position[lane] = vertex.position[lane];
            converted.normal[lane] = vertex.normal[lane];
        }
        converted.uv[0] = vertex.uv[0];
        converted.uv[1] = vertex.uv[1];
        converted.color = vertex.color;
        converted.light_color =
            -vertex.normal[1] * light.y - vertex.normal[0] * light.x - vertex.normal[2] * light.z < 0.0f ? 0u
                                                                                                         : 0xFFFFFFFFu;
        D3DXVECTOR3 direction{view.x - vertex.position[0], view.y - vertex.position[1], view.z - vertex.position[2]};
        D3DXVec3Normalize(&direction, &direction);
        D3DXVECTOR3 half{direction.x - light.x, direction.y - light.y, direction.z - light.z};
        D3DXVec3Normalize(&half, &half);
        converted.shine_uv[0] =
            (half.z * vertex.tangent[2] + half.y * vertex.tangent[1] + half.x * vertex.tangent[0] + 1.0f) * 0.5f;
        converted.shine_uv[1] =
            (half.z * vertex.binormal[2] + half.y * vertex.binormal[1] + half.x * vertex.binormal[0] + 1.0f) * 0.5f;
    }
    IDirect3DVertexBuffer9_Unlock(buffer);
}

template <bool Interior>
struct ShinyNode : nglShaderNode {
    nglMaterialBase *material;
    nglTexture *texture;

    ShinyNode(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
        : nglShaderNode(mesh, section), material(mat)
    {
        const auto &data = *reinterpret_cast<ShinyPayload<Interior> *>(mat);
        texture = data.texture;
        if ((texture->m_format & 0xFFu) == 16) {
            auto &params = mesh->field_8C;
            const uint32_t frame = params.IsSetParam<nglTextureFrameParam>()
                                       ? params.Get<nglTextureFrameParam>()->field_0
                                   : data.tod_frames() ? uint32_t(g_TOD) + 4u * nglCurScene->IFLFrame
                                                       : nglCurScene->IFLFrame;
            texture = texture->Frames[frame % texture->m_num_palettes];
        }
        static void *table[]{
            func_address(&ShinyNode::Render), func_address(&ShinyNode::GetSortInfo), func_address(&ShinyNode::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }

    void GetSortInfo(nglSortInfo &) {}
    ShinyNode *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Render()
    {
        static Var<int> suppress_interior{0x956FF0};
        if (Interior ? suppress_interior() : suppress_shiny())
            return;
        const auto &data = *reinterpret_cast<const ShinyPayload<Interior> *>(material);
        auto &state = g_renderState();
        state.setCullingMode(data.cull == 2 ? D3DCULL_NONE : D3DCULL_CW);
        vector4d localLight, localView;
        if (EnableShader || ChromeEffect) {
            const auto direction = shinyLightDirection(m_meshNode);
            const auto inverse = m_meshNode->sub_4199D0();
            localLight = shinyBasis({-direction.x, -direction.y, -direction.z, -direction.w}, inverse);
            localView = sub_414360(nglCurScene->ViewPos, inverse);
        }
        if (EnableShader) {
            nglSetVertexDeclarationAndShader(&shiny_programs().vertex[Interior ? 1 : 0]);
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal[0][0], 4);
            localLight.w = 0.0f;
            localView.w = 1.0f;
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 5, &localLight.x, 1);
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 6, &localView.x, 1);
        } else {
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[10]);
            if (ChromeEffect)
                shinyChromeVertices(m_meshSection, localLight, localView);
        }
        state.setBlending(static_cast<nglBlendModeType>(data.blend), 0, 128);
        color constant;
        if constexpr (Interior) {
            constant = {1, 1, 1, 1};
            auto &params = m_meshNode->field_8C;
            if (params.IsSetParam<nglTintParam>()) {
                const auto &tint = *params.Get<nglTintParam>()->field_0;
                constant = {tint.x, tint.y, tint.z, tint.w};
            }
        } else {
            sub_413850(material, &m_meshNode->field_8C, &constant);
        }
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &constant.r, 1);
        } else {
            const auto byte = [](float value) {
                return uint32_t(value * 255.0f) & 255u;
            };
            const uint32_t packed =
                byte(constant.b) | (byte(constant.g) << 8) | (byte(constant.r) << 16) | (byte(constant.a) << 24);
            if (state.field_9C != packed) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed);
                state.field_9C = packed;
            }
        }
        state.setBlending(std::not_equal_to<float>{}(constant.a, 1.0f) && data.blend <= 1
                              ? NGLBM_BLEND
                              : static_cast<nglBlendModeType>(data.blend),
                          0,
                          128);
        state.setColourBufferWriteEnabled(7);
        if (EnableShader) {
            SetPixelShader(&shiny_pixel_shader());
        } else if (ChromeEffect) {
            nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
            nglSetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_TEMP);
            nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_MODULATE);
            nglSetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
            nglSetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_TEMP | D3DTA_ALPHAREPLICATE);
            nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            nglSetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_MODULATE);
            nglSetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_CURRENT);
            nglSetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_TFACTOR | D3DTA_ALPHAREPLICATE);
            nglSetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(2, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
            nglSetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_MULTIPLYADD);
            nglSetTextureStageState(3, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            nglSetTextureStageState(3, D3DTSS_COLORARG2, D3DTA_TEMP);
            nglSetTextureStageState(3, D3DTSS_COLORARG0, D3DTA_CURRENT);
            nglSetTextureStageState(3, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
            nglSetTextureStageState(3, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
            nglSetTextureStageState(4, D3DTSS_COLOROP, D3DTOP_DISABLE);
            nglSetTextureStageState(4, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
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
        nglDxSetTexture(0, texture, 8, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        if (EnableShader || ChromeEffect) {
            nglDxSetTexture(1, data.shine, 2, 3);
            nglSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
            nglSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        }
        nglSetStreamSourceAndDrawPrimitive(m_meshSection);
        if (!EnableShader && ChromeEffect)
            nglSetTextureStageState(0, D3DTSS_RESULTARG, D3DTA_CURRENT);
    }
};
static_assert(sizeof(ShinyNode<false>) == 0x1C);

template <bool Interior = false, bool Morphable = false>
struct ShinyShader : nglShader {
    static void __fastcall Name(ShinyShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{Interior ? (Morphable ? "USMShinyMorphableInterior" : "USShinyInterior")
                                      : (Morphable ? "USMShinyMorphable" : "SMShiny")};
    }
    static bool __fastcall Switchable(ShinyShader *, void *)
    {
        return true;
    }
    ShinyShader()
    {
        static void *table[]{func_address(&ShinyShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&ShinyShader::Add),
                             func_address(&ShinyShader::Bind),
                             func_address(&ShinyShader::Release),
                             func_address(&ShinyShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&ShinyShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    ShinyShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        if (EnableShader) {
            if (!shiny_programs().vertex[0].field_0) {
                nglCreateVertexDeclarationAndShader(&shiny_programs().vertex[0], shiny_elements, shiny_vertex);
                nglCreateVertexDeclarationAndShader(&shiny_programs().vertex[1], shiny_elements, shiny_vertex_linear);
                CreatePixelShader(&shiny_pixel_shader(), shiny_pixel);
            }
        } else if (!dword_9738E0[10]) {
            IDirect3DDevice9_CreateVertexDeclaration(
                g_Direct3DDevice, ChromeEffect ? shiny_chrome_elements : shiny_fixed_elements, &dword_9738E0[10]);
        }
    }
    void Bind(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<ShinyPayload<Interior> *>(material);
        data.texture = nglLoadTexture(*data.texture_name);
        data.shine = nglLoadTexture(*data.shine_name);
    }
    void Release(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<ShinyPayload<Interior> *>(material);
        nglReleaseTexture(data.texture);
        data.texture = nullptr;
        nglReleaseTexture(data.shine);
        data.shine = nullptr;
    }
    void Rebase(nglMaterialBase *material, unsigned int offset)
    {
        auto &data = *reinterpret_cast<ShinyPayload<Interior> *>(material);
        if (data.texture_name)
            data.texture_name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(data.texture_name) + offset);
        if (data.shine_name)
            data.shine_name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(data.shine_name) + offset);
    }
    void Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
    {
        auto *node = new (nglListAlloc(sizeof(ShinyNode<Interior>), 16)) ShinyNode<Interior>{mesh, section, material};
        node->m_tex = reinterpret_cast<nglTexture *>(uint32_t(material->m_shader->field_8) << 24);
        node->m_next_node = nglCurScene->OpaqueNodes;
        nglCurScene->OpaqueNodes = node;
        ++nglCurScene->OpaqueListCount;
    }
};
static_assert(sizeof(ShinyShader<>) == 0xC);
}  // namespace

void initialize_shiny_material_shader()
{
    static ShinyShader<> shiny;
    static ShinyShader<true> shiny_interior;
    static ShinyShader<false, true> shiny_morphable;
    static ShinyShader<true, true> shiny_morphable_interior;
    initialize_outline_material_shaders();
}
#endif
static Var<IDirect3DPixelShader9 *> dword_970770{0x00970770};

static Var<VShader> stru_970760{0x00970760};

void sub_411830()
{
    int i = 0;

    //static Var<const DWORD *> off_939CD8{0x00939CD8};

    {
        static const char text[] = "tex t0\n"
                                   "mul r0, t0, c0\n";

        nglCreatePShader(&(&dword_970770())[i++], text);
    }

    {
        static const char text[] = "mov r0, c0\n";
        nglCreatePShader(&(&dword_970770())[i++], text);
    }
}

static Var<VShader> stru_970768{0x00970768};

static Var<IDirect3DPixelShader9 *> dword_970774{0x00970774};

double calc_outline_thickness(const math::VecClass<3, 1> &a1)
{
#if STANDALONE_SYSTEM
    const auto &view = nglCurScene->ViewPos;
    const float x = a1[0] - view.x, y = a1[1] - view.y, z = a1[2] - view.z;
    const double distance = std::sqrt(z * z + y * y + x * x);
    if (distance > 14.0)
        return 14.0 * 0.1f * 0.0025f;
    if (distance >= 0.5)
        return distance * (0.8f + 0.1f - 0.8f * distance * (1.0f / 14.0f)) * 0.0025f;
    return 0.5 * 0.8f * 0.0025f;
#else
    double (*func)(const void *) = CAST(func, 0x00406B60);
    return func(&a1);
#endif
}

namespace USOutlineShaderSpace {

template <>
void Outline_ShaderNode<USExteriorMaterial>::Render()
{
    TRACE("Outline_ShaderNode<USExteriorMaterial>::Render");

    static Var<int> dword_956FFC{0x00956FFC};
    if (dword_956FFC() == 0) {
        USExteriorMaterial *v2 = CAST(v2, this->field_14);
        auto v14 = v2->field_6C[0];
        auto v15 = v2->field_6C[1];
        auto v16 = v2->field_6C[2];
        this->sub_413AF0();
        auto v3 = this->m_meshNode->sub_4199D0();
        auto v17 = sub_414360(nglCurScene->ViewPos, v3);
        v17 = sub_414360(this->m_meshNode->Mesh->SphereCenter, this->m_meshNode->LocalToWorld);

        auto v4 = g_renderState().field_74;
        auto v5 = g_renderState().field_7C;
        g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
        if (g_renderState().field_A8 != 7) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_COLORWRITEENABLE, 7);
            g_renderState().field_A8 = 7;
        }

        if (g_renderState().field_7C != 4) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZFUNC, 4);
            g_renderState().field_7C = (D3DCMPFUNC)4;
        }

        if (!g_renderState().field_74) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZWRITEENABLE, 1);
            g_renderState().field_74 = 1;
        }

        if (g_renderState().m_cullingMode != D3DCULL_CW) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_CULLMODE, 2);
            g_renderState().m_cullingMode = D3DCULL_CW;
        }

        g_renderTextureState().setSamplerState(0, 8u, 3u);
        nglDxSetTexture(0, this->field_18, 8u, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        if (EnableShader) {
            nglSetVertexDeclarationAndShader(&stru_970760());
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &this->m_meshNode->WorldToLocal[0][0], 4);
        } else {
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice, (D3DTRANSFORMSTATETYPE)256, bit_cast<D3DMATRIX *>(&this->m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[9]);
        }

        color v18{};
        if (EnableShader) {
            SetPixelShader(&dword_970770());
            sub_413F80(&v18, this->field_14, &this->m_meshNode->field_8C, 9u);
            v17[0] = v18.r;
            v17[1] = v18.g;
            v17[2] = v18.b;
            v17[3] = v18.a;
            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, &v17[0], 1);
        } else {
            sub_413F80(&v18, this->field_14, &this->m_meshNode->field_8C, 9u);
            nglSetTextureStageState(0, D3DTSS_COLOROP, 4u);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_COLORARG2, 3u);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG2, 3u);
            nglSetTextureStageState(1u, D3DTSS_COLOROP, 1u);
            nglSetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
        }

        nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        if (g_renderState().field_7C != v5) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZFUNC, v5);

            g_renderState().field_7C = v5;
        }

        if (g_renderState().field_74 != v4) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZWRITEENABLE, v4);

            g_renderState().field_74 = v4;
        }

        if (v2->field_68 && EnableShader) {
            math::VecClass<3, 1> v17{};

            v17[0] = this->m_meshNode->LocalToWorld[3][0];
            v17[1] = this->m_meshNode->LocalToWorld[3][1];
            v17[2] = this->m_meshNode->LocalToWorld[3][2];
            v17[3] = this->m_meshNode->LocalToWorld[3][3];
            auto v13 = calc_outline_thickness(v17);
            if (v2->field_7C > 1.f) {
                v13 *= v2->field_7C;
            }

            if (g_renderState().field_A8 != 7) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_COLORWRITEENABLE, 7);
                g_renderState().field_A8 = 7;
            }

            if (g_renderState().m_cullingMode != D3DCULL_CCW) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_CULLMODE, 3);
                g_renderState().m_cullingMode = D3DCULL_CCW;
            }

            nglSetVertexDeclarationAndShader(&stru_970768());
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &this->m_meshNode->WorldToLocal[0][0], 4);

            {
                float v17[4]{};
                v17[3] = v13;
                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 8, v17, 1);
            }

            SetPixelShader(&dword_970774());

            {
                float v17[4]{};
                v17[0] = v14;
                v17[1] = v15;
                v17[2] = v16;
                v17[3] = 1.0;
                IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, v17, 1);
            }
            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        }
    }
}

vector4d sub_41BA70([[maybe_unused]] nglMaterialBase *a1, int *a2, unsigned int a4)
{
#if STANDALONE_SYSTEM
    auto &params = *reinterpret_cast<nglParamSet<nglShaderParamSet_Pool> *>(a2);
    vector4d result{1, 1, 1, 1};
    if (params.IsSetParam<nglTintParam>())
        result = *params.Get<nglTintParam>()->field_0;
    if (EnableShader) {
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, a4, &result.x, 1);
    } else {
        const auto byte = [](float value) {
            return uint32_t(value * 255.0f) & 255u;
        };
        const uint32_t packed =
            byte(result.z) | (byte(result.y) << 8) | (byte(result.x) << 16) | (byte(result.w) << 24);
        if (g_renderState().field_9C != packed) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, packed);
            g_renderState().field_9C = packed;
        }
    }
    g_renderState().setBlending(std::not_equal_to<float>{}(result.w, 1.0f) ? NGLBM_BLEND : NGLBM_OPAQUE, 0, 128);
    g_renderState().setColourBufferWriteEnabled(7);
    return result;
#else
    vector4d result;
    CDECL_CALL(0x0041BA70, &result, a1, a2, a4);
    return result;
#endif
}

template <>
void Outline_ShaderNode<USInteriorMaterial>::Render()
{
    static Var<int> dword_957000{0x00957000};
    if (!dword_957000()) {
        auto *v2 = this->field_14;
        auto v14 = v2->field_28[0];
        auto v15 = v2->field_28[1];
        auto v16 = v2->field_28[2];
        this->sub_413AF0();
        auto v3 = this->m_meshNode->sub_4199D0();
        auto v17 = sub_414360(nglCurScene->ViewPos, v3);
        v17 = sub_414360(this->m_meshNode->Mesh->SphereCenter, this->m_meshNode->LocalToWorld);
        auto v4 = g_renderState().field_74;
        auto v5 = g_renderState().field_7C;
        g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
        if (g_renderState().field_A8 != 7) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_COLORWRITEENABLE, 7);
            g_renderState().field_A8 = 7;
        }

        if (g_renderState().field_7C != 4) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZFUNC, 4);
            g_renderState().field_7C = (D3DCMPFUNC)4;
        }

        if (!g_renderState().field_74) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZWRITEENABLE, 1);
            g_renderState().field_74 = 1;
        }

        if (g_renderState().m_cullingMode != D3DCULL_CW) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_CULLMODE, 2);
            g_renderState().m_cullingMode = D3DCULL_CW;
        }

        g_renderTextureState().setSamplerState(0, 8u, 3u);
        nglDxSetTexture(0, this->field_18, 8u, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        if (EnableShader) {
            nglSetVertexDeclarationAndShader(&stru_970760());
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &this->m_meshNode->WorldToLocal[0][0], 4);
        } else {
            IDirect3DDevice9_SetTransform(g_Direct3DDevice,
                                          static_cast<D3DTRANSFORMSTATETYPE>(256),
                                          bit_cast<D3DMATRIX *>(&this->m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[9]);
        }

        if (EnableShader) {
            SetPixelShader(&dword_970770());
            auto v6 = sub_41BA70(this->field_14, (int *)&this->m_meshNode->field_8C, 9u);
            v17[0] = v6[0];
            v17[1] = v6[1];
            v17[2] = v6[2];
            v17[3] = v6[3];
            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, &v17[0], 1);
        } else {
            [[maybe_unused]] auto v18 = sub_41BA70(this->field_14, (int *)&this->m_meshNode->field_8C, 9u);
            nglSetTextureStageState(0, D3DTSS_COLOROP, 4u);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_COLORARG2, 3u);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG2, 3u);
            nglSetTextureStageState(1u, D3DTSS_COLOROP, 1u);
            nglSetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
        }

        nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        if (g_renderState().field_7C != v5) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZFUNC, v5);
            g_renderState().field_7C = v5;
        }

        if (g_renderState().field_74 != v4) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_ZWRITEENABLE, v4);
            g_renderState().field_74 = v4;
        }

        auto *v8 = this->field_14;
        if (v2->field_24 && EnableShader) {
            v17[0] = this->m_meshNode->LocalToWorld[3][0];
            v17[1] = this->m_meshNode->LocalToWorld[3][1];
            v17[2] = this->m_meshNode->LocalToWorld[3][2];
            v17[3] = this->m_meshNode->LocalToWorld[3][3];
            auto v13 = calc_outline_thickness(*bit_cast<math::VecClass<3, 1> *>(&v17));
            if (v8->field_38 > 1.f) {
                v13 *= v8->field_38;
            }

            if (g_renderState().field_A8 != 7) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_COLORWRITEENABLE, 7);
                g_renderState().field_A8 = 7;
            }

            if (g_renderState().m_cullingMode != D3DCULL_CCW) {
                IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_CULLMODE, 3);
                g_renderState().m_cullingMode = D3DCULL_CCW;
            }

            nglSetVertexDeclarationAndShader(&stru_970768());
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &this->m_meshNode->WorldToLocal[0][0], 4);

            v17[3] = v13;
            v17 = {};

            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 8, &v17[0], 1);
            SetPixelShader(&dword_970774());

            v17[0] = v14;
            v17[1] = v15;
            v17[2] = v16;
            v17[3] = 1.0;
            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, &v17[0], 1);
            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        }
    }
}

}  // namespace USOutlineShaderSpace

#if STANDALONE_SYSTEM
namespace {
constexpr DWORD outline_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x80000003,
    0x900F0002, 0x00000001, 0xD00F0001, 0xA000005B, 0x00000001, 0xC00F0001, 0xA0AA005B, 0x00000009, 0xC0010000,
    0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001, 0x00000009, 0xC0040000, 0x90E40000,
    0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000001, 0xE00F0000, 0x90E40001, 0x0000FFFF,
};
constexpr DWORD outline_expanded_vertex[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x80000003,
    0x900F0002, 0x00000001, 0xD00F0001, 0xA000005B, 0x00000001, 0xC00F0001, 0xA0AA005B, 0x00000001, 0x800F0000,
    0x90E40002, 0x00000004, 0x800F0002, 0x80A40000, 0xA0FF0008, 0x90E40000, 0x00000001, 0x80080002, 0x90FF0000,
    0x00000009, 0xC0010000, 0x80E40002, 0xA0E40000, 0x00000009, 0xC0020000, 0x80E40002, 0xA0E40001, 0x00000009,
    0xC0040000, 0x80E40002, 0xA0E40002, 0x00000009, 0xC0080000, 0x80E40002, 0xA0E40003, 0x0000FFFF,
};
constexpr D3DVERTEXELEMENT9 outline_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 20, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    D3DDECL_END(),
};
template <bool Interior>
struct OutlineMaterialShader : nglShader {
    using Payload = std::conditional_t<Interior, ShinyInteriorMaterial, ShinyMaterial>;
    using Node =
        USOutlineShaderSpace::Outline_ShaderNode<std::conditional_t<Interior, USInteriorMaterial, USExteriorMaterial>>;
    static void __fastcall Name(OutlineMaterialShader *, void *, tlFixedString *out)
    {
        *out = tlFixedString{Interior ? "USOutlineInterior" : "US_Outline"};
    }
    static bool __fastcall Switchable(OutlineMaterialShader *, void *)
    {
        return true;
    }
    static void __fastcall Sort(Node *, void *, nglSortInfo &) {}
    static Node *__fastcall DeleteNode(Node *node, void *, unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(node);
        return node;
    }
    OutlineMaterialShader()
    {
        static void *table[]{func_address(&OutlineMaterialShader::Register),
                             reinterpret_cast<void *>(Name),
                             func_address(&OutlineMaterialShader::Add),
                             func_address(&OutlineMaterialShader::Bind),
                             func_address(&OutlineMaterialShader::Release),
                             func_address(&OutlineMaterialShader::Rebase),
                             func_address(&nglShader::_CheckMaterialVersion),
                             func_address(&nglShader::_CheckVertexDefVersion),
                             func_address(&nglShader::_BindSection),
                             reinterpret_cast<void *>(Switchable),
                             func_address(&OutlineMaterialShader::Delete)};
        m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
    }
    OutlineMaterialShader *Delete(unsigned char flags)
    {
        if (flags & 1)
            ::operator delete(this);
        return this;
    }
    void Register()
    {
        nglShader::_Register();
        if (EnableShader) {
            if (!stru_970760().field_0) {
                nglCreateVertexDeclarationAndShader(&stru_970760(), outline_elements, outline_vertex);
                nglCreateVertexDeclarationAndShader(&stru_970768(), outline_elements, outline_expanded_vertex);
                sub_411830();
            }
        } else if (!dword_9738E0[9])
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, outline_elements, &dword_9738E0[9]);
    }
    void Bind(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<Payload *>(material);
        data.texture = nglLoadTexture(*data.texture_name);
    }
    void Release(nglMaterialBase *material)
    {
        auto &data = *reinterpret_cast<Payload *>(material);
        nglReleaseTexture(data.texture);
        data.texture = nullptr;
    }
    void Rebase(nglMaterialBase *material, unsigned int offset)
    {
        auto &name = reinterpret_cast<Payload *>(material)->texture_name;
        if (name)
            name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(name) + offset);
    }
    void Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *material)
    {
        const auto &data = *reinterpret_cast<Payload *>(material);
        auto *texture = data.texture;
        if ((texture->m_format & 0xFFu) == 16) {
            auto &params = mesh->field_8C;
            const uint32_t frame = params.IsSetParam<nglTextureFrameParam>()
                                       ? params.Get<nglTextureFrameParam>()->field_0
                                   : data.tod_frames() ? uint32_t(g_TOD) + 4u * nglCurScene->IFLFrame
                                                       : nglCurScene->IFLFrame;
            texture = texture->Frames[frame % texture->m_num_palettes];
        }
        auto *node = new (nglListAlloc(sizeof(Node), 16)) Node{nglShaderNode{mesh, section}, material, texture};
        static void *table[]{
            func_address(&Node::Render), reinterpret_cast<void *>(Sort), reinterpret_cast<void *>(DeleteNode)};
        node->m_vtbl = reinterpret_cast<decltype(node->m_vtbl)>(table);
        node->m_tex = reinterpret_cast<nglTexture *>(uint32_t(field_8) << 24);
        node->m_next_node = nglCurScene->OpaqueNodes;
        nglCurScene->OpaqueNodes = node;
        ++nglCurScene->OpaqueListCount;
    }
};
void initialize_outline_material_shaders()
{
    static OutlineMaterialShader<false> exterior;
    static OutlineMaterialShader<true> interior;
}
}
#endif

void us_outline_patch()
{
    REDIRECT(0x004118D9, sub_411830);

    {
        auto func = &USOutlineShaderSpace::Outline_ShaderNode<USExteriorMaterial>::Render;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871B24, address);
    }

    {
        auto func = &USOutlineShaderSpace::Outline_ShaderNode<USInteriorMaterial>::Render;
        FUNC_ADDRESS(address, func);
        set_vfunc(0x00871B30, address);
    }
}
