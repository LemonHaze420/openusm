#include "us_tentacle.h"

#include "common.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "nglshader.h"
#include "nglsortinfo.h"
#include "ngl_dx_shader.h"
#include "ngl_dx_state.h"
#include "ngl_dx_texture.h"
#include "polytubecustommaterial.h"
#include "utility.h"
#include "variables.h"

#include <cstddef>
#include <new>

namespace {
struct TentacleShader : nglShader {
    TentacleShader();
    void Register();
    tlFixedString Name() const;
    void Add(nglMeshNode *, nglMeshSection *, nglMaterialBase *);
    void Bind(nglMaterialBase *);
    void Release(nglMaterialBase *);
    void Rebase(nglMaterialBase *, uint32_t);
    TentacleShader *Delete(unsigned char);
};

struct TentacleNode : nglShaderNode {
    Tentacle_ShaderMaterial *material;
    TentacleNode(nglMeshNode *, nglMeshSection *, Tentacle_ShaderMaterial *);
    void Render();
    void GetSortInfo(nglSortInfo &);
    TentacleNode *Delete(unsigned char);
    void RenderFixedFunction();
};
VALIDATE_SIZE(TentacleShader, 0xC);
VALIDATE_SIZE(TentacleNode, 0x18);
static_assert(offsetof(Tentacle_ShaderMaterial, texture) == 0x20);
static_assert(offsetof(Tentacle_ShaderMaterial, sphere_map) == 0x28);
static_assert(offsetof(Tentacle_ShaderMaterial, enabled) == 0x2C);
static_assert(offsetof(Tentacle_ShaderMaterial, field_2E) == 0x2E);

struct VertexShaders {
    VShader shader[3];
};
Var<VertexShaders> vertex_shaders{0x00970C10};
Var<IDirect3DPixelShader9 *> pixel_texture{0x00956300};
Var<IDirect3DPixelShader9 *> pixel_main{0x009562E4};
Var<IDirect3DPixelShader9 *> pixel_color_map{0x009562C4};
Var<IDirect3DPixelShader9 *> pixel_color_map_vertex{0x009562F8};
Var<IDirect3DPixelShader9 *> pixel_map{0x009562D0};
Var<IDirect3DPixelShader9 *> pixel_map_vertex{0x009562E8};
Var<IDirect3DPixelShader9 *> pixel_map_vertex_alpha{0x009562D8};
Var<IDirect3DPixelShader9 *> pixel_normal_map{0x009562FC};
Var<IDirect3DPixelShader9 *> pixel_outline{0x00956304};
Var<IDirect3DPixelShader9 *> pixel_texture_alpha{0x009562F0};
Var<int> suppress_tentacle{0x00956FE8};
Var<int> outline_enabled{0x00956FE0};
Var<int> outline_ccw{0x00956FDC};
Var<float> last_intensity{0x00956FE4};
struct Intensity {
    float value = 1.0f;
};
struct MaterialIntensity {
    float value = 0.4f;
};
struct OutlineOffset {
    float value = 0.0002f;
};
Var<Intensity> vertex_intensity{0x0091E740};
Var<MaterialIntensity> enabled_intensity{0x0091E748};
Var<MaterialIntensity> disabled_intensity{0x0091E744};
Var<OutlineOffset> outline_offset{0x0091E73C};


const D3DVERTEXELEMENT9 vertex_elements[] = {
    {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
    {0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    D3DDECL_END(),
};
constexpr DWORD vertex_0[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x0000001F, 0x80000003, 0x900F0003, 0x00000001, 0xD00F0001, 0xA000005B, 0x00000001, 0xC00F0001,
    0xA0AA005B, 0x00000009, 0xC0010000, 0x90E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x90E40000, 0xA0E40001,
    0x00000009, 0xC0040000, 0x90E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x90E40000, 0xA0E40003, 0x00000008,
    0x80010001, 0x90E40003, 0xA0E40007, 0x00000008, 0x80020001, 0x90E40003, 0xA0E40008, 0x00000008, 0x80040001,
    0x90E40003, 0xA0E40009, 0x00000005, 0x80030001, 0x80540001, 0xA0E40004, 0x00000004, 0xE0030000, 0x80540001,
    0xA055005B, 0xA055005B, 0x00000002, 0x800F0000, 0xA0E4000B, 0x91E40000, 0x00000008, 0x80080002, 0x80A40000,
    0x80A40000, 0x00000007, 0x80080002, 0x80FF0002, 0x00000005, 0x80070000, 0x80A40000, 0x80FF0002, 0x00000008,
    0x800F000B, 0x90E40003, 0x80E40000, 0x00000002, 0x800F000B, 0x80E4000B, 0xA0E4000D, 0x00000005, 0x800F0003,
    0x80E4000B, 0xA0E4000C, 0x00000002, 0xD00F0000, 0x80E40003, 0x90E40002, 0x0000FFFF};
constexpr DWORD vertex_1[] = {0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001,
                              0x0000001F, 0x8000000A, 0x900F0002, 0x0000001F, 0x80000003, 0x900F0003, 0x00000001,
                              0xD00F0001, 0xA000005B, 0x00000001, 0xC00F0001, 0xA0AA005B, 0x00000001, 0x800F000B,
                              0x90E40003, 0x00000004, 0x800F0000, 0x80A4000B, 0xA0FF0005, 0x90E40000, 0x00000009,
                              0xC0010000, 0x80E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x80E40000, 0xA0E40001,
                              0x00000009, 0xC0040000, 0x80E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x80E40000,
                              0xA0E40003, 0x00000001, 0xD00F0000, 0xA0E40006, 0x0000FFFF};
constexpr DWORD vertex_2[] = {
    0xFFFE0101, 0x0000001F, 0x80000000, 0x900F0000, 0x0000001F, 0x80000005, 0x900F0001, 0x0000001F, 0x8000000A,
    0x900F0002, 0x0000001F, 0x80000003, 0x900F0003, 0x00000001, 0xD00F0001, 0xA000005B, 0x00000001, 0xC00F0001,
    0xA0AA005B, 0x00000001, 0x800F000B, 0x90E40003, 0x00000004, 0x800F0000, 0x80A4000B, 0xA0FF0005, 0x90E40000,
    0x00000009, 0xC0010000, 0x80E40000, 0xA0E40000, 0x00000009, 0xC0020000, 0x80E40000, 0xA0E40001, 0x00000009,
    0xC0040000, 0x80E40000, 0xA0E40002, 0x00000009, 0xC0080000, 0x80E40000, 0xA0E40003, 0x00000008, 0x80010001,
    0x90E40003, 0xA0E40007, 0x00000008, 0x80020001, 0x90E40003, 0xA0E40008, 0x00000008, 0x80040001, 0x90E40003,
    0xA0E40009, 0x00000008, 0x80080002, 0x80A40001, 0x80A40001, 0x00000007, 0x80080002, 0x80FF0002, 0x00000005,
    0x80070001, 0x80A40001, 0x80FF0002, 0x00000001, 0xE00F0000, 0x80540001, 0x0000FFFF};

Tentacle_ShaderMaterial *materialObject(nglMaterialBase *base)
{
    return base ? reinterpret_cast<Tentacle_ShaderMaterial *>(reinterpret_cast<char *>(base) - 4) : nullptr;
}

void __fastcall shader_name(TentacleShader *shader, void *, tlFixedString *out)
{
    *out = shader->Name();
}


bool __fastcall material_version(TentacleShader *, void *, nglMaterialBase *)
{
    return true;
}
bool __fastcall vertex_version(TentacleShader *, void *, nglMeshSection *)
{
    return true;
}
void __fastcall section_bind(TentacleShader *, void *, nglMeshSection *) {}
bool __fastcall switchable(TentacleShader *, void *)
{
    return false;
}

void setupSamplers()
{
    g_renderTextureState().setSamplerState(0, 8, 3);
    g_renderTextureState().setSamplerState(1, 8, 3);
    if (EnableShader)
        g_renderTextureState().setSamplerState(2, 8, 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    nglSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    nglSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    if (EnableShader) {
        nglSetSamplerState(2, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        nglSetSamplerState(2, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    }
}

TentacleShader::TentacleShader()
{
    static void *table[] = {
        func_address(&TentacleShader::Register),
        reinterpret_cast<void *>(shader_name),
        func_address(&TentacleShader::Add),
        func_address(&TentacleShader::Bind),
        func_address(&TentacleShader::Release),
        func_address(&TentacleShader::Rebase),
        reinterpret_cast<void *>(material_version),
        reinterpret_cast<void *>(vertex_version),
        reinterpret_cast<void *>(section_bind),
        reinterpret_cast<void *>(switchable),
        func_address(&TentacleShader::Delete),
    };
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}

tlFixedString TentacleShader::Name() const
{
    return tlFixedString{"US_Object"};
}

void TentacleShader::Register()
{
    nglShader::_Register();
    if (EnableShader) {
        const DWORD *programs[] = {vertex_0, vertex_1, vertex_2};
        for (int i = 0; i < 3; ++i)
            nglCreateVertexDeclarationAndShader(&vertex_shaders().shader[i], vertex_elements, programs[i]);


        nglCreatePShader(&pixel_texture(), "tex t0\nmov r0.rgb, t0\n+mov r0.a, c1.a\n");
        nglCreatePShader(&pixel_main(), "tex t0\nmul r0.rgb, v0, t0\n+mov r0.a, c1.a\n");
        nglCreatePShader(&pixel_color_map(),
                         "tex t0\ntex t1\nmul r0, t0, c0\nmad r0, r0, t1.a, r0\nadd r0, r0, t1\nmov r0.a, c1.a\n");
        nglCreatePShader(&pixel_color_map_vertex(),
                         "tex t0\ntex t1\nmul r0, t0, c0\nmad r0, t0, t1.a, r0\nmad r0, v0, t0.a, r0\nmad r0, t1, "
                         "t0.a, r0\nmov r0.a, c1.a\n");
        nglCreatePShader(&pixel_map(), "tex t0\ntex t1\nmul r0, t0, t1.a\nadd r0, r0, t1\nmov r0.a, c1.a\n");
        nglCreatePShader(
            &pixel_map_vertex(),
            "tex t0\ntex t1\nmul r0, t0, t1.a\nmad r0, v0, t0.a, r0\nmad r0, t1, t0.a, r0\nmov r0.a, c1.a\n");
        nglCreatePShader(&pixel_map_vertex_alpha(),
                         "tex t0\ntex t1\nmul r0, t0, t1.a\nadd r0, r0, t1\nmul r1, v0, t0.a\nmad r0, r1, t1.a, "
                         "r0\nmov r0.a, c1.a\n");
        nglCreatePShader(
            &pixel_normal_map(),
            "tex t0\ntex t1\ntex t2\ndp3_sat r0, t1_bx2, t2_bx2\nmul r0, t0, r0.a\nmul r0, r0, c1\nmul r0, r0, t1.a\n");
        nglCreatePShader(&pixel_outline(), "mov r0, v0\nmov r0.a, c0.a\n");
        nglCreatePShader(&pixel_texture_alpha(), "tex t0\nmov r0, t0\nmov r0.a, c0.a\n");
    } else if (!dword_9738E0[13]) {
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, vertex_elements, &dword_9738E0[13]);
    }
}

void TentacleShader::Bind(nglMaterialBase *base)
{
    auto *mat = materialObject(base);
    mat->texture = nglLoadTexture(*mat->texture_name);
    mat->sphere_map = nglLoadTexture(*mat->sphere_map_name);
}

void TentacleShader::Release(nglMaterialBase *base)
{
    auto *mat = materialObject(base);
    nglReleaseTexture(mat->texture);
    nglReleaseTexture(mat->sphere_map);
    mat->texture = nullptr;
    mat->sphere_map = nullptr;
}

void TentacleShader::Rebase(nglMaterialBase *base, uint32_t offset)
{
    auto *mat = materialObject(base);
    if (mat->texture_name)
        mat->texture_name = reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(mat->texture_name) + offset);
    if (mat->sphere_map_name)
        mat->sphere_map_name =
            reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(mat->sphere_map_name) + offset);
}

TentacleShader *TentacleShader::Delete(unsigned char flags)
{
    if (flags & 1)
        ::operator delete(this);
    return this;
}

TentacleNode::TentacleNode(nglMeshNode *mesh, nglMeshSection *section, Tentacle_ShaderMaterial *mat)
    : nglShaderNode(mesh, section), material(mat)
{
    static void *table[] = {func_address(&TentacleNode::Render),
                            func_address(&TentacleNode::GetSortInfo),
                            func_address(&TentacleNode::Delete)};
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
}

void TentacleShader::Add(nglMeshNode *mesh, nglMeshSection *section, nglMaterialBase *mat)
{
    auto *storage = nglListAlloc(sizeof(TentacleNode), 16);
    if (storage)
        nglListAddNode(new (storage) TentacleNode{mesh, section, materialObject(mat)});
    else
        nglListAddNode(nullptr);
}

void TentacleNode::GetSortInfo(nglSortInfo &info)
{
    info.Type = NGLSORT_OPAQUE;
    info.u = getTentacle_Shader().field_8;
}

TentacleNode *TentacleNode::Delete(unsigned char flags)
{
    if (flags & 1)
        ::operator delete(this);
    return this;
}

void TentacleNode::RenderFixedFunction()
{
    sub_413AF0();
    setupSamplers();
    auto &state = g_renderState();
    state.setCullingMode(D3DCULL_CCW);
    state.setBlending(NGLBM_OPAQUE, 0, 0);
    IDirect3DDevice9_SetTransform(
        g_Direct3DDevice, D3DTS_WORLD, reinterpret_cast<const D3DMATRIX *>(&m_meshNode->LocalToWorld));
    IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[13]);
    const auto depthWrite = state.field_74;
    const auto depthFunction = state.field_7C;
    state.setColourBufferWriteEnabled(15);
    state.setDepthBufferFunction(D3DCMP_LESSEQUAL);
    state.setDepthBufferWriteEnabled(true);
    state.setCullingMode(D3DCULL_CCW);
    if (state.field_9C != 0xFF646464u) {
        IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, 0xFF646464u);
        state.field_9C = 0xFF646464u;
    }
    nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
    nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
    nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    nglSetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    nglSetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    state.setDepthBufferFunction(depthFunction);
    state.setDepthBufferWriteEnabled(depthWrite);
}

void TentacleNode::Render()
{
    if (suppress_tentacle())
        return;
    if (!EnableShader) {
        RenderFixedFunction();
        return;
    }
    sub_413AF0();
    setupSamplers();
    auto &state = g_renderState();
    state.setCullingMode(D3DCULL_CCW);
    state.setBlending(NGLBM_OPAQUE, 0, 0);


    (void)m_meshNode->sub_4199D0();
    const float materialIntensity = material->enabled ? enabled_intensity().value : disabled_intensity().value;
    const vector4d pixelColor{materialIntensity, materialIntensity, materialIntensity, 1.0f};
    const vector4d zero{0.0f, 0.0f, 0.0f, 0.0f};
    const vector4d one{1.0f, 1.0f, 1.0f, 1.0f};
    const auto depthFunction = state.field_7C;
    const auto depthWrite = state.field_74;
    state.setColourBufferWriteEnabled(7);
    state.setDepthBufferFunction(D3DCMP_LESSEQUAL);
    state.setDepthBufferWriteEnabled(true);
    state.setCullingMode(D3DCULL_CCW);
    nglDxSetTexture(0, material->texture, 8, 3);
    nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    nglDxSetTexture(1, material->sphere_map, 8, 3);
    nglSetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
    nglSetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
    nglSetVertexDeclarationAndShader(&vertex_shaders().shader[0]);
    IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal.arr[0].x, 4);


    ptr_to_po pair{&m_meshNode->LocalToWorld, &nglCurScene->WorldToView};
    vector4d x, y, z, position;
    pair.build_world_basis_and_pos(x, y, z, position);
    const vector4d viewConstants[] = {
        {x.x, y.x, z.x, position.x},
        {x.y, y.y, z.y, position.y},
        {x.z, y.z, z.z, position.z},
        {0.0f, 0.0f, 0.0f, 1.0f},
    };
    IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 7, &viewConstants[0].x, 4);
    IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 11, &zero.x, 1);
    IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 12, &zero.x, 1);
    const float intensity = vertex_intensity().value;
    last_intensity() = intensity;
    const vector4d vertexColor{intensity, intensity, intensity, 1.0f};
    IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &vertexColor.x, 1);
    SetPixelShader(&pixel_main());
    IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, &pixelColor.x, 1);
    IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 1, &one.x, 1);
    nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    state.setDepthBufferFunction(depthFunction);
    state.setDepthBufferWriteEnabled(depthWrite);
    if (outline_enabled() && material->field_2E) {
        state.setColourBufferWriteEnabled(7);
        state.setCullingMode(outline_ccw() ? D3DCULL_CCW : D3DCULL_CW);
        nglSetVertexDeclarationAndShader(&vertex_shaders().shader[1]);
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &m_meshNode->WorldToLocal.arr[0].x, 4);
        const vector4d displacement{0.0f, 0.0f, 0.0f, outline_offset().value};
        const vector4d black{0.0f, 0.0f, 0.0f, 1.0f};
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 5, &displacement.x, 1);
        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 6, &black.x, 1);
        SetPixelShader(&pixel_outline());
        IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, &black.x, 1);
        nglSetStreamSourceAndDrawPrimitive(m_meshSection);
    }
    state.setColourBufferWriteEnabled(7);
}
}  // namespace

nglShader &getTentacle_Shader()
{
    static Var<TentacleShader> shader{0x0091E62C};
    return shader();
}
