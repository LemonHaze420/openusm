#include "us_person.h"
#include "us_native_shader_programs.h"

#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>

#include "comic_panels.h"
#include "common.h"
#include "func_wrapper.h"
#include "light_source.h"
#include "log.h"
#include "matrix4x3.h"
#include "mstring.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "nglsortinfo.h"
#include "tl_system.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"
#include "vector3d.h"

#include <ngl_dx_scene.h>

#include <d3d9.h>
#include <d3dx9tex.h>

#include <cassert>
#include <filesystem>

Var<int> USPersonParam::ID{0x0095678C};

namespace USPersonShaderSpace {

VALIDATE_SIZE(USPersonNode, 0x28);
VALIDATE_SIZE(USPersonSolidNode, 0x28);
VALIDATE_SIZE(USPersonNode::LightInfoStruct, 0x38);
VALIDATE_SIZE(USPersonShader, 0x2C);
VALIDATE_SIZE(USPersonSolidShader, 0x2C);
VALIDATE_OFFSET(USPersonShader, field_C, 0xC);
VALIDATE_OFFSET(USPersonSolidShader, field_C, 0xC);

VALIDATE_OFFSET(ParamStruct, field_30, 0x30);

Var<USPersonNode::LightInfoStruct> USPersonNode::DefaultLightInfo{0x0091E750};

Var<ParamStruct> DefaultParams{0x00956918};

namespace {
const bool person_defaults_initialized = [] {
#if STANDALONE_SYSTEM
    auto &params = DefaultParams();
    params = {};
    params.field_0[3] = params.field_10[3] = params.field_20[3] = 1.0f;
    params.field_30 = 1.0f;
    params.field_38 = true;
    for (auto address : {0x0091E750u, 0x0091E788u}) {
        Var<USPersonNode::LightInfoStruct> info{static_cast<ptrdiff_t>(address)};
        info() = {};
        auto *ambient = reinterpret_cast<float *>(&info().field_20);
        for (int i = 0; i < 4; ++i)
            ambient[i] = 1.0f;
        info().m_contrast = 1.0f;
    }
#endif
    return true;
}();
}  // namespace

static Var<IDirect3DPixelShader9 *> OutlinePShader{0x009707F0};

#if STANDALONE_SYSTEM
namespace {
template <typename Shader>
void __fastcall person_shader_name(Shader *shader, void *, tlFixedString *out)
{
    *out = shader->field_C;
}
bool __fastcall person_material_version(nglShader *, void *, nglMaterialBase *)
{
    return true;
}
bool __fastcall person_vertex_version(nglShader *, void *, nglMeshSection *)
{
    return true;
}
void __fastcall person_bind_section(nglShader *, void *, nglMeshSection *) {}
bool __fastcall person_switchable(nglShader *, void *)
{
    return false;
}
}  // namespace
#endif

USPersonShader::USPersonShader() : field_C{"USPerson"}
{
#if STANDALONE_SYSTEM
    static void *table[]{
        func_address(&USPersonShader::_Register),
        reinterpret_cast<void *>(person_shader_name<USPersonShader>),
        func_address(&USPersonShader::_AddNode),
        func_address(&USPersonShader::_BindMaterial),
        func_address(&USPersonShader::_ReleaseMaterial),
        func_address(&USPersonShader::_RebaseMaterial),
        reinterpret_cast<void *>(person_material_version),
        reinterpret_cast<void *>(person_vertex_version),
        reinterpret_cast<void *>(person_bind_section),
        reinterpret_cast<void *>(person_switchable),
        func_address(&USPersonShader::Delete),
    };
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
#endif
}

USPersonSolidShader::USPersonSolidShader() : field_C{"USPersonSolid"}
{
#if STANDALONE_SYSTEM
    static void *table[]{
        func_address(&USPersonSolidShader::Register),
        reinterpret_cast<void *>(person_shader_name<USPersonSolidShader>),
        func_address(&USPersonSolidShader::_AddNode),
        func_address(&USPersonSolidShader::_BindMaterial),
        func_address(&USPersonSolidShader::_ReleaseMaterial),
        func_address(&USPersonSolidShader::_RebaseMaterial),
        reinterpret_cast<void *>(person_material_version),
        reinterpret_cast<void *>(person_vertex_version),
        reinterpret_cast<void *>(person_bind_section),
        reinterpret_cast<void *>(person_switchable),
        func_address(&USPersonSolidShader::Delete),
    };
    m_vtbl = reinterpret_cast<decltype(m_vtbl)>(table);
#endif
}

tlFixedString USPersonSolidShader::_GetName()
{
    return field_C;
}

USPersonShader &getUSPersonShader()
{
    static Var<USPersonShader> shader{0x0095683C};
    return shader();
}
USPersonSolidShader &getUSPersonSolidShader()
{
    static Var<USPersonSolidShader> shader{0x009567E4};
    return shader();
}
USPersonShader &getUSPersonMorphableShader()
{
    static auto &shader = []() -> USPersonShader & {
        auto &instance = var<USPersonShader>(0x00956810);
        instance.field_C = tlFixedString{"USPersonMorphable"};
        return instance;
    }();
    return shader;
}
USPersonShader &getUSPersonNickFuryEyeShader()
{
    static auto &shader = []() -> USPersonShader & {
        auto &instance = var<USPersonShader>(0x009567B8);
        instance.field_C = tlFixedString{"USPersonMorphable_NickFuryEye"};
        return instance;
    }();
    return shader;
}

static Var<VShader[2]> OutlineVShader{0x009707E0};

static Var<VShader[1]> stru_9707E8{0x009707E8};

static Var<VShader[4]> g_vertexShaders{0x009707F4};

static Var<IDirect3DPixelShader9 *[8]> g_pixelShaders{0x009707C0};

void CreateVertexDeclAndShaders(const D3DVERTEXELEMENT9 *elements)
{
#if STANDALONE_SYSTEM
    using namespace us_native_programs;
    const DWORD *programs[]{program_8ae4a0, program_8ae6b0, program_8ae968, program_8aec08};
    for (unsigned i = 0; i < 4; ++i) {
        nglCreateVertexDeclarationAndShader(&g_vertexShaders()[i], elements, programs[i]);
    }
#else
    if constexpr (1) {
        static Var<const DWORD *[4]> g_pFunctions{0x00939D20};

        [[maybe_unused]] auto **v1 = g_pFunctions();
        auto *array_shaders = g_vertexShaders();

        if constexpr (0) {
#include "../../shaders/us_person/0_VS.h"

            nglCreateVShader(elements, &array_shaders[0], 0, text);

            assert(compare_codes(v1[0], g_codes, size_codes(v1[0])));
        } else {
            D3DXMACRO defines[] = {{"USE_LIGHTING", "0"}, {nullptr, nullptr}};
            auto shader = CompileVShader("shaders/us_person/1_VS.hlsl", defines);

            nglCreateVertexDeclarationAndShader(&array_shaders[0], elements, shader.data());
        }

        if constexpr (0) {
#include "../../shaders/us_person/1_VS.h"

            nglCreateVShader(elements, &array_shaders[1], 0, text);

            //assert(compare_codes(v1[1], g_codes, size_codes(v1[1])));
        } else {
            D3DXMACRO defines[] = {{"USE_LIGHTING", "1"}, {nullptr, nullptr}};
            auto shader = CompileVShader("shaders/us_person/1_VS.hlsl", defines);

            nglCreateVertexDeclarationAndShader(&array_shaders[1], elements, shader.data());
        }

        if constexpr (1) {
#include "../../shaders/us_person/2_VS.h"

            nglCreateVShader(elements, &array_shaders[2], 0, text);

            assert(compare_codes(v1[2], g_codes, size_codes(v1[2])));
        } else {
            D3DXMACRO defines[] = {{"USE_LIGHTING", "0"}, {nullptr, nullptr}};
            auto shader = CompileVShader("shaders/us_person/3_VS.hlsl", defines);

            nglCreateVertexDeclarationAndShader(&array_shaders[2], elements, shader.data());
        }

        if constexpr (1) {
#include "../../shaders/us_person/3_VS.h"

            nglCreateVShader(elements, &array_shaders[3], 0, text);

            assert(compare_codes(v1[3], g_codes, size_codes(v1[3])));
        } else {
            D3DXMACRO defines[] = {{"USE_LIGHTING", "1"}, {nullptr, nullptr}};

            auto shader = CompileVShader("shaders/us_person/3_VS.hlsl", defines);

            nglCreateVertexDeclarationAndShader(&array_shaders[3], elements, shader.data());
        }

    } else {
        CDECL_CALL(0x004114D0, elements);
    }
#endif
}

void CreatePixelShaders()
{
#if STANDALONE_SYSTEM
    using namespace us_native_programs;
    const DWORD *programs[]{program_8af3d8,
                            program_8af404,
                            program_8af430,
                            program_8af478,
                            program_8af4c0,
                            program_8af508,
                            program_8af550,
                            program_8af598};
    for (unsigned i = 0; i < 8; ++i) {
        CreatePixelShader(&g_pixelShaders()[i], programs[i]);
    }
#else
    if constexpr (0) {
        if constexpr (0) {
            static Var<const DWORD *[8]> functions{0x00939D38};

            for (int i = 0; i < 8; ++i) {
                CreatePixelShader(&g_pixelShaders()[i], functions()[i]);

                //sp_log("%s", disassemble_shader(off_939D38()[i]));
            }

        } else {
            static const char *functions[8] = {"tex t0\n"
                                               "mul r0.xyz, c2, t0\n"
                                               "+mov r0.w, t0.w\n",

                                               "tex t0\n"
                                               "mul_x2 r0.xyz, v0, t0\n"
                                               "+mov r0.w, t0.w\n",

                                               "tex t0\n"
                                               "tex t1\n"
                                               "mul r0.xyz, c2, t0\n"
                                               "+mov r0.w, t0.w\n"
                                               "mad r0.xyz, r0, t1.w, c1\n",

                                               "tex t0\n"
                                               "tex t1\n"
                                               "mul_x2 r0.xyz, v0, t0\n"
                                               "+mov r0.w, t0.w\n"
                                               "mad r0.xyz, r0, t1.w, c1\n",

                                               "tex t0\n"
                                               "tex t1\n"
                                               "mul r0.xyz, c2, t0\n"
                                               "+mov r0.w, t0.w\n"
                                               "mad r0.xyz, r0, c1.w, t1\n",

                                               "tex t0\n"
                                               "tex t1\n"
                                               "mul_x2 r0.xyz, v0, t0\n"
                                               "+mov r0.w, t0.w\n"
                                               "mad r0.xyz, r0, c1.w, t1\n",

                                               "tex t0\n"
                                               "tex t1\n"
                                               "mul r0.xyz, c2, t0\n"
                                               "+mov r0.w, t0.w\n"
                                               "mad r0.xyz, r0, t1.w, t1\n",

                                               "tex t0\n"
                                               "tex t1\n"
                                               "mul_x2 r0.xyz, v0, t0\n"
                                               "+mov r0.w, t0.w\n"
                                               "mad r0.xyz, r0, t1.w, t1\n"};

            static Var<const DWORD *[4]> g_pFunctions{0x00939D38};

            [[maybe_unused]] auto **v1 = g_pFunctions();

            for (int i = 0; i < 8; ++i) {
                if (i == 1) {
                    auto shader = CompilePShader("shaders/us_person/1_PS.hlsl");
                    CreatePixelShader(&g_pixelShaders()[i], shader.data());

                    continue;
                } else if (i == 7) {
                    auto shader = CompilePShader("shaders/us_person/7_PS.hlsl");
                    CreatePixelShader(&g_pixelShaders()[i], shader.data());

                    continue;
                }
                nglCreatePShader(&g_pixelShaders()[i], functions[i]);
            }
        }
    } else {
        CDECL_CALL(0x00411550);
    }
#endif
}

void CreateOutlineVShader(const D3DVERTEXELEMENT9 *elements)
{
#if STANDALONE_SYSTEM
    using namespace us_native_programs;
    nglCreateVertexDeclarationAndShader(&OutlineVShader()[0], elements, program_8aeef8);
    nglCreateVertexDeclarationAndShader(&OutlineVShader()[1], elements, program_8af168);
#else
    TRACE("CreateOutlineVShader");

    if constexpr (1) {
        static Var<const DWORD *[2]> off_939D30{0x00939D30};

        auto &v1 = off_939D30();
        auto &v2 = OutlineVShader();

        if constexpr (1) {
#include "../../shaders/us_person/outline_VS.h"

            nglCreateVShader(elements, &v2[0], 0, text);
            nglCreateVShader(elements, &v2[1], 0, text);

            assert(compare_codes(v1[1], g_codes, size_codes(v1[1])));
            assert(compare_codes(v1[0], v1[1], size_codes(v1[1])));

            //sp_log("%s", disassemble_shader(v1[0]));
        } else {
            auto shader = CompileVShader("shaders/us_person/outline_VS.hlsl");

            nglCreateVertexDeclarationAndShader(&v2[0], elements, shader.data());
            nglCreateVertexDeclarationAndShader(&v2[1], elements, shader.data());
        }

    } else {
        CDECL_CALL(0x00411510, elements);
    }
#endif
}

void *ParamStruct::operator new(size_t size)
{
    auto *mem = nglListAlloc(size, 16);
    return mem;
}

USPersonSolidNode::USPersonSolidNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4)
    : USPersonNode(a2, a3, a4)
{
    static void *table[]{func_address(&USPersonSolidNode::_Render), func_address(&USPersonSolidNode::_GetSortInfo)};
    m_vtbl = CAST(m_vtbl, table);
}

void *USPersonSolidNode::operator new(size_t size)
{
    auto *mem = nglListAlloc(size, 16);
    return mem;
}

namespace {
void add_solid_pass(nglMeshNode *source, nglMeshSection *section, nglMaterialBase *material, int pass)
{
    auto *mesh = static_cast<nglMeshNode *>(nglListAlloc(sizeof(nglMeshNode), 64));
    ::new (mesh) nglMeshNode{*source};
    nglParamSet<nglShaderParamSet_Pool> copied{static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};
    copied.copy(source->field_8C);
    if (pass != 1)
        copied.Set<USSectionIFLParam>();
    auto *params =
        new ParamStruct{source->field_8C.IsSetParam<USPersonParam>() ? *source->field_8C.Get<USPersonParam>()->field_0
                                                                     : DefaultParams()};
    params->field_44 = pass;
    if (pass != 1) {
        params->field_38 = false;
        params->field_3C = pass == 2 ? 1 : 2;
        params->field_41 = false;
        if (pass == 2)
            params->disableZDepth = true;
        else {
            std::memcpy(params->field_0, params->field_10, sizeof(params->field_0));
            params->field_30 = params->field_34;
        }
    }
    copied.SetParam(USPersonParam{params});
    mesh->field_8C = copied;
    nglListAddNode(new USPersonSolidNode{mesh, section, material});
}
}  // namespace

void USPersonSolidShader::sub_41DEE0(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    add_solid_pass(a1, a2, a3, 1);
}

void USPersonSolidShader::sub_41E0A0(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    add_solid_pass(a1, a2, a3, 2);
}

void USPersonSolidShader::sub_41E290(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    add_solid_pass(a1, a2, a3, 3);
}

void USPersonSolidShader::_AddNode(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    TRACE("USPersonSolidShader::AddNode");
    if constexpr (STANDALONE_SYSTEM) {
        auto *v9 = &USPersonShaderSpace::DefaultParams();
        if (a1->field_8C.IsSetParam<USPersonParam>()) {
            auto *param = a1->field_8C.Get<USPersonParam>();

            v9 = param->field_0;
        }

        if ((reinterpret_cast<USPersonMaterial *>(a3)->field_38 || v9->field_48) &&
            (comic_panels::get_panel_params() == nullptr || (comic_panels::get_panel_params()->field_D1 & 1) == 0)) {
            this->sub_41DEE0(a1, a2, a3);
            this->sub_41E0A0(a1, a2, a3);
        } else {
            auto *v8 = new USPersonSolidNode{a1, a2, a3};
            nglListAddNode(v8);
        }

        if (v9->field_40) {
            this->sub_41E290(a1, a2, a3);
        }
    } else {
        THISCALL(0x0041DBE0, this, a1, a2, a3);
    }
}

void USPersonSolidShader::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USPersonShaderSpace::USPersonSolidShader::BindMaterial");

    USPersonMaterial *Material = CAST(Material, a1);

#ifdef TARGET_XBOX
    a1->field_1C = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_18));
    a1->field_24 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_20));
#else
    Material->field_1C = nglLoadTexture(*Material->field_18);
    Material->field_24 = nglLoadTexture(*Material->field_20);
#endif
}


void USPersonSolidShader::_RebaseMaterial(nglMaterialBase *a1, unsigned int a2)
{
    TRACE("USPersonSolidShader::RebaseMaterial");

#ifndef TARGET_XBOX
    auto *material = reinterpret_cast<USPersonMaterial *>(a1);
    PTR_OFFSET(a2, material->field_18);
    PTR_OFFSET(a2, material->field_20);
#endif
}

#if STANDALONE_SYSTEM
namespace {
void register_person_shader(nglShader &shader)
{
    shader.nglShader::_Register();
    if (EnableShader) {
        static constexpr D3DVERTEXELEMENT9 skin_elements[]{
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
            {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
            {0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
            D3DDECL_END(),
        };
        CreateVertexDeclAndShaders(skin_elements);
        CreatePixelShaders();
        CreateOutlineVShader(skin_elements);
        CreatePixelShader(&OutlinePShader(), us_native_programs::program_8af5e0);
    } else if (dword_9738E0[12] == nullptr) {
        static constexpr D3DVERTEXELEMENT9 fixed_elements[]{
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, 20, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
            D3DDECL_END(),
        };
        IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, fixed_elements, &dword_9738E0[12]);
    }
}
}  // namespace
#endif

void USPersonSolidShader::Register()
{
#if STANDALONE_SYSTEM
    register_person_shader(*this);
#else
    sp_log("USPersonSolidShader::Register:");

    if constexpr (0) {
        nglShader::Register();

        D3DVERTEXELEMENT9 nglVS_Skin_Decl[]{
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
            {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
            {0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
            D3DDECL_END()

        };

        if (EnableShader) {
            CreateVertexDeclAndShaders(nglVS_Skin_Decl);
            CreatePixelShaders();
            CreateOutlineVShader(nglVS_Skin_Decl);

            {
                auto pShader = CompilePShader("shaders/us_person/outline_PS.hlsl");

                CreatePixelShader(&OutlinePShader(), pShader.data());
            }

        } else {
            D3DVERTEXELEMENT9 nglVS_NonSkin_Decl4[]{
                {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
                {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                {0, 20, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                D3DDECL_END()};

            static Var<IDirect3DVertexDeclaration9 *> dword_973910{0x00973910};

            if (dword_973910() == nullptr) {
                IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, nglVS_NonSkin_Decl4, &dword_973910());
            }
        }
    } else {
        THISCALL(0x00411580, this);
    }
#endif
}

void USPersonShader::_Register()
{
#if STANDALONE_SYSTEM
    register_person_shader(*this);
#else
    TRACE("USPersonShader::Register");

    if constexpr (1) {
        nglShader::Register();

        if (EnableShader) {
            static const D3DVERTEXELEMENT9 nglVS_Skin_Decl[] = {
                {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
                {0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
                {0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                {0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0},
                {0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
                D3DDECL_END()};

            CreateVertexDeclAndShaders(nglVS_Skin_Decl);
            CreatePixelShaders();

            CreateOutlineVShader(nglVS_Skin_Decl);

            {
                auto pShader = CompilePShader("shaders/us_person/outline_PS.hlsl");
                CreatePixelShader(&OutlinePShader(), pShader.data());
            }
        } else {
            D3DVERTEXELEMENT9 nglVS_NonSkin_Decl[]{
                {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
                {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                {0, 20, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
                D3DDECL_END()};

            if (dword_9738E0[12] == nullptr) {
                IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, nglVS_NonSkin_Decl, &dword_9738E0[12]);
            }
        }
    } else {
        THISCALL(0x00411580, this);
    }
#endif
}

void USPersonShader::AddNodeOverrideMask(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *new_node = static_cast<nglMeshNode *>(nglListAlloc(sizeof(nglMeshNode), 64));
        ::new (new_node) nglMeshNode{*a1};

        nglParamSet<nglShaderParamSet_Pool> param_set{
            static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};
        param_set.copy(a1->field_8C);
        new_node->field_8C = param_set;

        auto *v7 = &DefaultParams();
        if (a1->field_8C.IsSetParam<USPersonParam>()) {
            v7 = a1->field_8C.Get<USPersonParam>()->field_0;
        }

        auto *v8 = new ParamStruct{};
        memcpy(v8, v7, sizeof(ParamStruct));

        v8->field_44 = 1;

        param_set.SetParam(USPersonParam{v8});

        auto *v12 = new USPersonNode{new_node, a2, a3};
        nglListAddNode(v12);
    } else {
        THISCALL(0x0041BEF0, this, a1, a2, a3);
    }
}

void USPersonShader::AddNodeClearZ(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *new_node = static_cast<nglMeshNode *>(nglListAlloc(sizeof(nglMeshNode), 64));
        ::new (new_node) nglMeshNode{*a2};
        nglParamSet<nglShaderParamSet_Pool> param_set{
            static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};

        param_set.copy(a2->field_8C);

        param_set.Set<USSectionIFLParam>();

        new_node->field_8C = param_set;

        auto *v9 = &DefaultParams();
        if (a2->field_8C.IsSetParam<USPersonParam>()) {
            v9 = a2->field_8C.Get<USPersonParam>()->field_0;
        }

        auto *v10 = new ParamStruct{};
        std::memcpy(v10, v9, sizeof(ParamStruct));

        v10->field_38 = false;
        v10->field_3C = 1;
        v10->field_41 = false;
        v10->field_44 = 2;
        v10->disableZDepth = true;

        param_set.SetParam(USPersonParam{v10});

        auto *v14 = new USPersonNode{new_node, a3, a4};
        nglListAddNode(v14);
    } else {
        THISCALL(0x0041C0B0, this, a2, a3, a4);
    }
}

void USPersonShader::AddNodeExtraOutline(nglMeshNode *a1, nglMeshSection *a2, nglMaterialBase *a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *new_node = static_cast<nglMeshNode *>(nglListAlloc(sizeof(nglMeshNode), 64));
        ::new (new_node) nglMeshNode{*a1};

        nglParamSet<nglShaderParamSet_Pool> param_set{
            static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};
        param_set.copy(a1->field_8C);

        param_set.Set<USSectionIFLParam>();

        new_node->field_8C = param_set;

        auto *a1a = &DefaultParams();
        if (a1->field_8C.IsSetParam<USPersonParam>()) {
            a1a = a1->field_8C.Get<USPersonParam>()->field_0;
        }

        auto *v7 = new ParamStruct{};
        memcpy(v7, a1a, sizeof(ParamStruct));

        v7->field_38 = false;
        v7->field_3C = 2;
        v7->field_0[0] = a1a->field_10[0];
        v7->field_0[1] = a1a->field_10[1];
        v7->field_0[2] = a1a->field_10[2];
        v7->field_0[3] = a1a->field_10[3];
        v7->field_30 = a1a->field_34;
        v7->field_41 = false;
        v7->field_44 = 3;

        param_set.SetParam(USPersonParam{v7});

        auto *v11 = new USPersonNode{new_node, a2, a3};
        nglListAddNode(v11);
    } else {
        THISCALL(0x0041C2A0, this, a1, a2, a3);
    }
}

void USPersonShader::_AddNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4)
{
    TRACE("USPersonShader::AddNode");

    if constexpr (STANDALONE_SYSTEM) {
        auto *v9 = &USPersonShaderSpace::DefaultParams();
        if (a2->field_8C.IsSetParam<USPersonParam>()) {
            auto *param = a2->field_8C.Get<USPersonParam>();
            v9 = param->field_0;
        }

        USPersonMaterial *Material = CAST(Material, a4);
        if ((Material->field_38 || v9->field_48) &&
            (comic_panels::get_panel_params() == nullptr || (comic_panels::get_panel_params()->field_D1 & 1) == 0)) {
            this->AddNodeOverrideMask(a2, a3, a4);
            this->AddNodeClearZ(a2, a3, a4);
        } else {
            auto *v8 = new USPersonNode{a2, a3, a4};
            nglListAddNode(v8);
        }

        if (v9->field_40) {
            this->AddNodeExtraOutline(a2, a3, a4);
        }
    } else {
        THISCALL(0x0041BBC0, this, a2, a3, a4);
    }
}

void USPersonShader::_BindMaterial(nglMaterialBase *a1)
{
    TRACE("USPersonShaderSpace::USPersonShader::BindMaterial");

    USPersonMaterial *Material = CAST(Material, a1);

#ifdef TARGET_XBOX
    a1->field_1C = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_18));
    a1->field_24 = nglLoadTexture(*bit_cast<tlHashString *>(&a1->field_20));
#else
    Material->field_1C = nglLoadTexture(*Material->field_18);
    Material->field_24 = nglLoadTexture(*Material->field_20);
#endif
}


void USPersonShader::_RebaseMaterial(nglMaterialBase *a1, unsigned int Base)
{
    TRACE("USPersonShaderSpace::USPersonShader::RebaseMaterial");

#ifndef TARGET_XBOX
    if constexpr (STANDALONE_SYSTEM) {
        USPersonMaterial *Material = CAST(Material, a1);

        PTR_OFFSET(Base, Material->field_18);
        PTR_OFFSET(Base, Material->field_20);
    } else {
        THISCALL(0x00410C60, this, a1, Base);
    }
#endif
}

tlFixedString USPersonShader::_GetName()
{
    tlFixedString result = this->field_C;

    return result;
}

void USPersonSolidNode::_Render()
{
    RenderPerson(true);
}

void USPersonSolidNode::_GetSortInfo(nglSortInfo &sortInfo)
{
    auto *v4 = &this->m_meshNode->field_8C;
    auto *v5 = &USPersonShaderSpace::DefaultParams();
    if (v4->IsSetParam<USPersonParam>()) {
        v5 = v4->Get<USPersonParam>()->field_0;
    }

    if (v5->disableZDepth) {
        sortInfo.Type = NGLSORT_TRANSLUCENT;
        sortInfo.Dist = -1.0e10;
    } else if (this->m_material->m_blend_mode < 2u) {
        sortInfo.Type = NGLSORT_OPAQUE;
        sortInfo.u = ((v5->field_44 & 2) << 29) | (this->m_material->m_shader->field_8 << 24) | 0x80000000;
    } else {
        sortInfo.Type = NGLSORT_TRANSLUCENT;
        sortInfo.Dist = this->sub_415D10();
    }
}

vector4d sub_4139A0(const vector4d *a2, const matrix4x4 *a3)
{
    return (*a3)[0] * (*a2)[0] + (*a3)[1] * (*a2)[1] + (*a3)[2] * (*a2)[2];
}

USPersonNode::USPersonNode(nglMeshNode *a2, nglMeshSection *a3, nglMaterialBase *a4) : USVariantShaderNode(a2, a3)
{
    static void *table[]{func_address(&USPersonNode::_Render), func_address(&USPersonNode::_GetSortInfo)};
    m_vtbl = CAST(m_vtbl, table);
    this->m_material = CAST(m_material, a4);
    this->field_24 = this->GetDistanceScale();
    this->field_1C = this->ResolveIFL(this->m_material->field_1C);
    this->field_20 = this->ResolveIFL(this->m_material->field_24);
}

void *USPersonNode::operator new(size_t size)
{
    auto *mem = nglListAlloc(size, 16);
    return mem;
}

bool USPersonNode::GetLightInfo(USPersonNode::LightInfoStruct &lightInfo)
{
    return GetLightInfo(lightInfo, false);
}

bool USPersonNode::GetLightInfo(USPersonNode::LightInfoStruct &lightInfo, bool solid)
{
    if constexpr (1) {
        bool result = false;

        auto *v4 = &this->m_meshNode->field_8C;
        if (v4->IsSetParam<USLightParam>()) {
            auto *param = v4->Get<USLightParam>();

            auto *v5 = param->field_0;

            vector3d v8 =
                sub_411750(*(vector4d *)&this->m_meshNode->Mesh->SphereCenter, this->m_meshNode->LocalToWorld[3]);
            v5->get_colors(v8, lightInfo.field_10, lightInfo.field_20);

            lightInfo.m_dir = v5->get_dir(v8);
            lightInfo.m_dir.w = 1.0;
            lightInfo.m_contrast = v5->properties->m_contrast;
            lightInfo.m_flags = v5->properties->m_flags;
            result = true;
        } else {
            Var<LightInfoStruct> defaults{solid ? 0x0091E788 : 0x0091E750};
            lightInfo = defaults();

            result = false;
        }

        //sp_log("lightInfoIsSet =  %d", result);

        return result;
    } else {
        auto result = (bool)THISCALL(0x0041CF70, this, &lightInfo);

        return result;
    }
}

static Var<IDirect3DVertexBuffer9 *> dword_973BC0{0x00973BC0};

static Var<uint32_t> dword_973BC4{0x00973BC4};

void USPersonNode::RenderWithDisableShader()
{
    RenderWithDisableShader(false);
}

void USPersonNode::RenderWithDisableShader(bool solid)
{
    if constexpr (1) {
        this->sub_413AF0();
        this->sub_413AF0();
        auto &v3 = this->m_meshNode->field_8C;
        auto *v4 = &USPersonShaderSpace::DefaultParams();
        if (v3.IsSet(USPersonParam::ID())) {
            v4 = v3.Get<USPersonParam>()->field_0;
        }

        auto v5 = v4->field_41;
        auto v14 = v4->field_38;
        auto v7 = v4->field_44;
        auto v15 = v5;
        if (v7) {
            g_renderState().setStencilCheckEnabled(true);
            g_renderState().setStencilRefValue(0x80u);
            g_renderState().setStencilFailOperation(1);
            g_renderState().setStencilDepthFailOperation(1);

            switch (v7) {
            case 1:
                g_renderState().setStencilBufferTestFunction(D3DCMP_ALWAYS);
                g_renderState().setStencilBufferWriteMask(0x80);
                g_renderState().setStencilPassOperation(D3DSTENCILOP_REPLACE);
                break;
            case 2:
                g_renderState().setStencilBufferTestFunction(D3DCMP_EQUAL);
                g_renderState().setStencilBufferCompareMask(0x80u);
                g_renderState().setStencilPassOperation(D3DSTENCILOP_KEEP);
                break;
            case 3:
                g_renderState().setStencilBufferTestFunction(D3DCMP_NOTEQUAL);
                g_renderState().setStencilBufferCompareMask(0x80u);
                g_renderState().setStencilPassOperation(D3DSTENCILOP_KEEP);
                break;
            }
        }

        IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[12]);

        D3DMATRIX v20{};
        memset(&v20._34, 0, 16);
        memset(&v20._23, 0, 16);
        memset(&v20._12, 0, 16);
        v20._44 = 1.0;
        v20._33 = 1.0;
        v20._22 = 1.0;
        v20._11 = 1.0;

        IDirect3DDevice9_SetTransform(g_Direct3DDevice, D3DTS_WORLD, &v20);

        auto *v13 = this->m_meshNode;

        D3DXVECTOR3 v19{};
        v19[0] = 0.0;
        v19[1] = 0.57735026;
        v19[2] = 0.81649655;

        D3DXVec3TransformNormal(&v19, &v19, bit_cast<D3DXMATRIX *>(&v13->LocalToWorld));

        if (nglSkinPersonMeshDX(this->m_meshNode, this->m_meshSection, v19) != nullptr) {
            if (v14) {
                nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

                nglSetTextureStageState(1u, D3DTSS_COLOROP, D3DTOP_MODULATE);
                nglSetTextureStageState(1u, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                nglSetTextureStageState(1u, D3DTSS_COLORARG2, D3DTA_CURRENT);
                nglSetTextureStageState(1u, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                nglSetTextureStageState(1u, D3DTSS_ALPHAARG1, D3DTA_CURRENT);

                nglSetTextureStageState(2u, D3DTSS_COLOROP, D3DTOP_DISABLE);
                nglSetTextureStageState(2u, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

                g_renderState().setColourBufferWriteEnabled(D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
                                                            D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

                g_renderState().setDepthBufferFunction(D3DCMP_LESSEQUAL);

                g_renderState().setDepthBufferWriteEnabled(true);

                g_renderState().setDepthBuffer(D3DZB_TRUE);

                g_renderState().setCullingMode(D3DCULL_CW);

                g_renderState().setBlending(this->m_material->m_blend_mode, 0, 0);
                nglDxSetTexture(0, this->field_1C, 8u, 3);
                nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
                const string_hash v16{int(this->m_meshSection->Material->Name->m_hash)};
                const string_hash v17{0x7A6B6091};
                const string_hash v18{0x7BB44A0E};

                if (v16 == v17 || (v16 == v18)) {
                    IDirect3DDevice9_SetTexture(g_Direct3DDevice, 1, celshadingSolidTex());
                    g_renderTextureState().field_0[1] = (IDirect3DTexture9 *)celshadingTex();
                } else {
                    IDirect3DDevice9_SetTexture(g_Direct3DDevice, 1, celshadingTex());
                    g_renderTextureState().field_0[1] = (IDirect3DTexture9 *)celshadingSolidTex();
                }

                nglSetSamplerState(1u, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                nglSetSamplerState(1u, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
                auto *v11 = this->m_meshSection;
                nglSetStreamSourceAndDrawPrimitive(v11->m_primitiveType,
                                                   dword_973BC0(),
                                                   v11->NVertices,
                                                   dword_973BC4(),
                                                   v11->m_stride,
                                                   v11->m_indexBuffer,
                                                   v11->NIndices,
                                                   v11->StartIndex);
            }

            if (v15) {
                g_renderState().setCullingMode(D3DCULL_CW);

                g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
                if (g_renderState().field_9C != 0xFF000000) {
                    IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_TEXTUREFACTOR, 0xFF000000);
                    g_renderState().field_9C = 0xFF000000;
                }

                nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
                nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);

                nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
                nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);

                Var<bool> byte_95701C{solid ? 0x00957038 : 0x0095701C};
                if (byte_95701C()) {
                    g_renderTextureState().field_0[0] = nullptr;
                    IDirect3DDevice9_SetTexture(g_Direct3DDevice, 0, nullptr);
                }

                auto *v12 = this->m_meshSection;
                nglSetStreamSourceAndDrawPrimitive(v12->m_primitiveType,
                                                   dword_973BC0(),
                                                   v12->NVertices,
                                                   dword_973BC4(),
                                                   v12->m_stride,
                                                   v12->m_indexBuffer,
                                                   v12->NIndices,
                                                   v12->StartIndex);
            }

            if (v7) {
                g_renderState().setStencilCheckEnabled(false);

                g_renderState().setStencilBufferTestFunction(D3DCMP_ALWAYS);

                g_renderState().setStencilPassOperation(D3DSTENCILOP_KEEP);
            }
        }
    } else {
        THISCALL(0x0041D180, this);
    }
}

void USPersonNode::_Render()
{
    RenderPerson(false);
}

void USPersonNode::RenderPerson(bool solid)
{
    if constexpr (STANDALONE_SYSTEM) {
        Var<int> dword_957018{solid ? 0x00957034 : 0x00957018};

        if (dword_957018()) {
            return;
        }

        if (!EnableShader) {
            this->RenderWithDisableShader(solid);
            return;
        }

        auto &v2 = this->m_meshNode->field_8C;
        const auto *params = &USPersonShaderSpace::DefaultParams();
        if (v2.IsSetParam<USPersonParam>()) {
            params = v2.Get<USPersonParam>()->field_0;
        }

        bool v4 = params->field_38;
        bool disableOutline = params->field_41;
        bool v41 = (v4 && (this->m_material->field_3C || this->m_material->field_40));

        LightInfoStruct lightInfo;

        bool lightParamIsSet = (v4 && this->m_material->field_44 != 0 && this->GetLightInfo(lightInfo, solid));

        bool enableOutline = (params->field_3C != 0 ? (params->field_3C == 2) : this->m_material->m_outlineFeature);

        uint32_t v49 = params->field_44;

        bool clearZTest = params->disableZDepth;

        IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &this->m_meshNode->WorldToLocal[0][0], 4);
        nglSetupVShaderBonesDX(11, this->m_meshNode, this->m_meshSection);

        if (v49 != 0) {
            g_renderState().setStencilCheckEnabled(true);

            g_renderState().setStencilRefValue(0x80u);

            g_renderState().setStencilFailOperation(1);

            g_renderState().setStencilDepthFailOperation(1);

            switch (v49) {
            case 1: {
                g_renderState().setStencilBufferTestFunction(D3DCMP_ALWAYS);
                g_renderState().setStencilBufferWriteMask(0x80u);
                g_renderState().setStencilPassOperation(D3DSTENCILOP_REPLACE);
            } break;
            case 2: {
                g_renderState().setStencilBufferTestFunction(D3DCMP_EQUAL);
                g_renderState().setStencilBufferCompareMask(0x80u);
                g_renderState().setStencilPassOperation(D3DSTENCILOP_KEEP);
            } break;
            case 3: {
                g_renderState().setStencilBufferTestFunction(D3DCMP_NOTEQUAL);
                g_renderState().setStencilBufferCompareMask(0x80u);
                g_renderState().setStencilPassOperation(D3DSTENCILOP_KEEP);
            } break;
            default:
                break;
            }
        }

        if (v4) {
            nglDxSetTexture(0, this->field_1C, 8, 3);
            nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
            nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

            if (v41) {
                nglDxSetTexture(1u, this->field_20, 8, 3);
                nglSetSamplerState(1u, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
                nglSetSamplerState(1u, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
            }

            g_renderState().setBlending(this->m_material->m_blend_mode, 0, 0);

            g_renderState().setCullingMode(D3DCULL_CW);

            if (v41) {
                matrix4x4 v12 = this->m_meshNode->sub_41D840();

                matrix4x3 v59 = sub_413770(v12);

                matrix4x4 v53;
                v53.sub_415650(v59);

                v53[0][3] = 1.0;

                v53[0] *= 0.5f;

                v53[1][3] = -1.0;

                v53[1] *= -0.5f;

                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 4, &v53[0][0], 2);
            }

            if (lightParamIsSet) {
                auto *material = this->m_material;

                vector4d a2 = material->field_28;

                vector4d a3 = (*bit_cast<vector4d *>(&lightInfo.field_20)) * 0.5f;

                a2 *= a3;

                a3 = vector4d{1.0};

                vector4d a1 = a3 - a2;

                a3 = *bit_cast<vector4d *>(&lightInfo.field_10) * a1;

                if ((lightInfo.m_flags & 2) == 0) {
                    a1 = *bit_cast<vector4d *>(&lightInfo.field_20);
                    a3 *= a1;
                }

                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 8, &a2[0], 1);

                a1 = -lightInfo.m_dir;

                matrix4x4 v39 = this->m_meshNode->sub_4199D0();

                vector4d v27 = a1 * lightInfo.m_contrast;

                vector4d v28 = sub_4139A0(&v27, &v39);

                vector4d v53 = v28.sub_41CF30();
                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 6, &v53[0], 1);

                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 7, &a3[0], 1);
            }

            {
                auto idx = lightParamIsSet + 2 * v41;

                nglSetVertexDeclarationAndShader(&g_vertexShaders()[idx]);
            }

            {
                auto idx = lightParamIsSet + 2 * (this->m_material->field_40 + 2 * this->m_material->field_3C);

                SetPixelShader(&g_pixelShaders()[idx]);
            }

            if (!lightParamIsSet) {
                vector4d a1 = this->m_material->field_28;

                IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 2, &a1[0], 1);
            }

            float a1[4]{0, 0, 0, 1};

            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 1, a1, 1);

            //sp_log("DRAW!");
            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        }

        if (disableOutline) {
            g_renderState().setCullingMode(D3DCULL_CW);

            g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);

            float a1[4]{0, 0, 0, 0};
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 9, a1, 1);

            nglSetVertexDeclarationAndShader(&OutlineVShader()[0]);
            SetPixelShader(&OutlinePShader());

            IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, params->field_20, 1);

            g_renderTextureState().field_0[0] = nullptr;
            IDirect3DDevice9_SetTexture(g_Direct3DDevice, 0, nullptr);

            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        }

        if (enableOutline) {
            auto v34 = this->GetDistanceScale();
            if (v34 > 0.0f) {
                g_renderState().setCullingMode(D3DCULL_CCW);
                g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
                auto v35 = v34 * params->field_30 * ParamStruct::OutlineThickness;

                float a1[4]{0, 0, 0, v35};
                IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 9, a1, 1);

                nglSetVertexDeclarationAndShader(&OutlineVShader()[0]);
                SetPixelShader(&OutlinePShader());

                IDirect3DDevice9_SetPixelShaderConstantF(g_Direct3DDevice, 0, params->field_0, 1);

                nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
            }
        }

        if (clearZTest) {
            g_renderState().setCullingMode(D3DCULL_CW);

            g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);

            g_renderState().setColourBufferWriteEnabled(0);

            g_renderState().setDepthBufferWriteEnabled(true);

            g_renderState().setDepthBufferFunction(D3DCMP_ALWAYS);

            float a1[4]{0, 0, 0, 0};
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 9, a1, 1);

            static float dword_957004[4]{0, 0, 0, 16777214.0};
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 10, dword_957004, 1);
            nglSetVertexDeclarationAndShader(&OutlineVShader()[1]);
            SetPixelShader(&OutlinePShader());

            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
            g_renderState().setColourBufferWriteEnabled(nglCurScene->FBWriteMask);

            g_renderState().setDepthBufferWriteEnabled(false);

            g_renderState().setDepthBufferFunction(D3DCMP_LESSEQUAL);
        }

        if (v49 != 0) {
            g_renderState().setStencilCheckEnabled(false);

            g_renderState().setStencilBufferTestFunction(D3DCMP_ALWAYS);

            g_renderState().setStencilPassOperation(D3DSTENCILOP_KEEP);
        }

    } else {
        THISCALL(0x0041C4C0, this);
    }
}

void USPersonNode::_GetSortInfo(nglSortInfo &sortInfo)
{
    TRACE("USPersonNode::GetSortInfo");

    auto *v4 = &this->m_meshNode->field_8C;
    auto *params = &USPersonShaderSpace::DefaultParams();
    if (v4->IsSetParam<USPersonParam>()) {
        params = v4->Get<USPersonParam>()->field_0;
    }

    if (params->disableZDepth) {
        sortInfo.Type = NGLSORT_TRANSLUCENT;
        sortInfo.Dist = -1.0e10;
    } else if (this->m_material->m_blend_mode < 2u) {
        sortInfo.Type = NGLSORT_OPAQUE;
        uint32_t v2 = ((params->field_44 & 0x2) << 29) | (this->m_material->m_shader->field_8 << 24) | 0x80000000;
        sortInfo.u = v2;
    } else {
        sortInfo.Type = NGLSORT_TRANSLUCENT;
        sortInfo.Dist = this->sub_415D10();
    }
}

void USPersonShader::_ReleaseMaterial(nglMaterialBase *base)
{
    auto *material = reinterpret_cast<USPersonMaterial *>(base);
    nglReleaseTexture(material->field_1C);
    nglReleaseTexture(material->field_24);
    material->field_1C = material->field_24 = nullptr;
}

void USPersonSolidShader::_ReleaseMaterial(nglMaterialBase *base)
{
    auto *material = reinterpret_cast<USPersonMaterial *>(base);
    nglReleaseTexture(material->field_1C);
    nglReleaseTexture(material->field_24);
    material->field_1C = material->field_24 = nullptr;
}

USPersonShader *USPersonShader::Delete(unsigned char flags)
{
    if (flags & 1)
        ::operator delete(this);
    return this;
}

USPersonSolidShader *USPersonSolidShader::Delete(unsigned char flags)
{
    if (flags & 1)
        ::operator delete(this);
    return this;
}

}  // namespace USPersonShaderSpace

void hookSetStreamSourceAndDrawPrimitive(nglMeshSection *) {}

void *_sub_4150E0(matrix4x4 *out, const matrix4x4 *a2)
{
    *out = sub_4150E0(*a2);
    return out;
}

void *_sub_4135B0(matrix4x3 *out, const matrix4x3 &a2)
{
    *out = transposed(a2);
    return out;
}


void us_person_patch()
{
    REDIRECT(0x0041510A, _sub_4135B0);
    {
        REDIRECT(0x00419A11, _sub_4150E0);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonShader::_Register);
        set_vfunc(0x008717DC, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonNode::_GetSortInfo);
        set_vfunc(0x00871D20, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonShader::_BindMaterial);
        set_vfunc(0x008717E8, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonShader::_RebaseMaterial);
        set_vfunc(0x008717F0, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonSolidShader::_BindMaterial);
        set_vfunc(0x00871814, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonSolidShader::_RebaseMaterial);
        set_vfunc(0x0087181C, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonNode::_Render);
        set_vfunc(0x00871D1C, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonSolidNode::_Render);
        set_vfunc(0x00871D3C, address);
    }

    {
        FUNC_ADDRESS(address,
                     static_cast<void (USPersonShaderSpace::USPersonNode::*)()>(
                         &USPersonShaderSpace::USPersonNode::RenderWithDisableShader));
        REDIRECT(0x0041C4EF, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonShader::_AddNode);
        set_vfunc(0x008717E4, address);
    }

    {
        FUNC_ADDRESS(address, &USPersonShaderSpace::USPersonSolidShader::_AddNode);
        set_vfunc(0x00871810, address);
    }

    {
        FUNC_ADDRESS(address, &USVariantShaderNode::GetDistanceScale);
        REDIRECT(0x0041BD15, address);
    }

    {
        REDIRECT(0x0041C5CE, nglSetupVShaderBonesDX);
        REDIRECT(0x0041E5BE, nglSetupVShaderBonesDX);
    }

    REDIRECT(0x0041171C, USPersonShaderSpace::CreatePixelShaders);

    return;

    //USPersonNode::Render;
    {
        {
            FUNC_ADDRESS(address,
                         static_cast<bool (USPersonShaderSpace::USPersonNode::*)(
                             USPersonShaderSpace::USPersonNode::LightInfoStruct &)>(
                             &USPersonShaderSpace::USPersonNode::GetLightInfo));
            REDIRECT(0x0041C56A, address);
        }

        {
            FUNC_ADDRESS(address, &USVariantShaderNode::GetDistanceScale);
            REDIRECT(0x0041CC36, address);
        }

        REDIRECT(0x0041CA54, nglSetVertexDeclarationAndShader);
    }

    //REDIRECT(0x0041CB31, hookSetStreamSourceAndDrawPrimitive);

    {
        FUNC_ADDRESS(address, &RenderState_t::setCullingMode);
        REDIRECT(0x0041CC57, address);
    }
}
