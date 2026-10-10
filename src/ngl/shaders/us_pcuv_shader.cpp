#include "us_pcuv_shader.h"
#include "us_native_shader_programs.h"

#include "femanager.h"
#include "func_wrapper.h"
#include "log.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "tl_system.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>

#include "common.h"

static Var<IDirect3DPixelShader9 *> PCUV_PShader{0x009562C8};

static Var<VShader> dword_970AD0{0x00970AD0};

VALIDATE_SIZE(PCUV_ShaderMaterial, 0x30u);

PCUV_ShaderMaterial::PCUV_ShaderMaterial()
{
    this->field_1C = nullptr;
    this->m_texture = nullptr;
    this->m_blend_mode = static_cast<nglBlendModeType>(0);
    this->field_28 = 2;
    this->field_2C = 0;
    this->field_8 = &gPCUV_Shader();
}

PCUV_ShaderMaterial::PCUV_ShaderMaterial(nglTexture *a2, nglBlendModeType a3, int a4, int a5)
{
    this->m_texture = a2;
    this->m_blend_mode = a3;

    this->field_28 = a5;
    this->field_2C = a4;
    this->field_8 = &gPCUV_Shader();
}

void __fastcall PCUV_Shader_GetName(PCUV_Shader *self, void *, tlFixedString *out)
{
    *out = self->_GetName();
}

bool __fastcall PCUV_Shader_IsSwitchable(void *)
{
    return false;
}

PCUV_Shader::PCUV_Shader()
{
    static void *g_vtbl[]{
        func_address(&PCUV_Shader::Register),
        reinterpret_cast<void *>(PCUV_Shader_GetName),
        func_address(&PCUV_Shader::_AddNode),
        func_address(&PCUV_Shader::_BindMaterial),
        func_address(&PCUV_Shader::_ReleaseMaterial),
        func_address(&PCUV_Shader::_RebaseMaterial),
        func_address(&nglShader::_CheckMaterialVersion),
        func_address(&nglShader::_CheckVertexDefVersion),
        func_address(&nglShader::_BindSection),
        reinterpret_cast<void *>(PCUV_Shader_IsSwitchable),
    };
    this->m_vtbl = CAST(m_vtbl, &g_vtbl);
}

PCUV_Shader &getPCUV_Shader()
{
    if constexpr (STANDALONE_SYSTEM) {
        return gPCUV_Shader();
    } else {
        static PCUV_Shader shader;
        return shader;
    }
}

tlFixedString PCUV_Shader::_GetName() const
{
    return tlFixedString{"US_PCUV"};
}

void PCUV_Shader::_AddNode(nglMeshNode *mesh_node, nglMeshSection *mesh_section, nglMaterialBase *material)
{
    auto *pcuv_material =
        material == nullptr
            ? nullptr
            : reinterpret_cast<PCUV_ShaderMaterial *>(reinterpret_cast<char *>(material) - sizeof(std::intptr_t));
    auto *node =
        new (nglListAlloc(sizeof(PCUV_ShaderNode), 16)) PCUV_ShaderNode{mesh_node, mesh_section, pcuv_material};
    if (pcuv_material->m_blend_mode <= NGLBM_PUNCHTHROUGH) {
        node->field_4 = reinterpret_cast<int>(nglCurScene->OpaqueNodes);
        nglCurScene->OpaqueNodes = reinterpret_cast<nglShaderNode *>(node);
        ++nglCurScene->OpaqueListCount;
    } else {
        sub_417C10(reinterpret_cast<nglShaderNode *>(node));
    }
}

void PCUV_Shader::_BindMaterial(nglMaterialBase *material)
{
    auto *pcuv_material =
        reinterpret_cast<PCUV_ShaderMaterial *>(reinterpret_cast<char *>(material) - sizeof(std::intptr_t));
    pcuv_material->m_texture = nglLoadTexture(*pcuv_material->field_1C);
}

void PCUV_Shader::_ReleaseMaterial(nglMaterialBase *material)
{
    auto *pcuv_material =
        reinterpret_cast<PCUV_ShaderMaterial *>(reinterpret_cast<char *>(material) - sizeof(std::intptr_t));
    nglReleaseTexture(pcuv_material->m_texture);
    pcuv_material->m_texture = nullptr;
}

void PCUV_Shader::_RebaseMaterial(nglMaterialBase *material, unsigned int base)
{
    auto *pcuv_material =
        reinterpret_cast<PCUV_ShaderMaterial *>(reinterpret_cast<char *>(material) - sizeof(std::intptr_t));
    if (pcuv_material->field_1C != nullptr) {
        pcuv_material->field_1C =
            reinterpret_cast<tlFixedString *>(reinterpret_cast<char *>(pcuv_material->field_1C) + base);
    }
}

PCUV_ShaderNode::PCUV_ShaderNode(nglMeshNode *mesh_node, nglMeshSection *mesh_section, PCUV_ShaderMaterial *material)
    : field_4(0), field_8(0), field_C(mesh_node), field_10(mesh_section), field_14(material)
{
    static void *g_vtbl[]{func_address(&PCUV_ShaderNode::Render)};
    this->m_vtbl = CAST(m_vtbl, &g_vtbl);
}

void PCUV_Shader::Register()
{
    if constexpr (1) {
        nglShader::_Register();

#if STANDALONE_SYSTEM
        static D3DVERTEXELEMENT9 vertex_elements[] = {
            {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END(),
        };
        auto *elements = vertex_elements;
#else
        static Var<D3DVERTEXELEMENT9> stru_91E1B4{0x0091E1B4};
        auto *elements = &stru_91E1B4();
#endif

        static Var<IDirect3DVertexDeclaration9 *> dword_973918{0x00973918};

        if (EnableShader) {
            auto vertexShader = CompileVShader("shaders/us_pcuv_VS.hlsl");

            //static Var<DWORD *> off_939FB0{0x00939FB0};
            nglCreateVertexDeclarationAndShader(&dword_970AD0(), elements, vertexShader.data());

            if constexpr (STANDALONE_SYSTEM) {
                static const char *text = "tex t0\n"
                                          "mul r0, t0, v0\n";

                nglCreatePShader(&PCUV_PShader(), text);
            } else {
                auto pShader = CompilePShader("shaders/us_pcuv_PS.hlsl");
                CreatePixelShader(&PCUV_PShader(), pShader.data());
            }

        } else if (dword_973918() == nullptr) {
            IDirect3DDevice9_CreateVertexDeclaration(g_Direct3DDevice, elements, &dword_973918());
        }
    } else {
        THISCALL(0x00402BC0, this);
    }
}

void PCUV_ShaderNode::Render()
{
    static Var<int> dword_956D34{0x00956D34};

    if (dword_956D34() == 0) {
        auto &params = this->field_C->field_8C;
        if (params.IsSetParam<nglTextureFrameParam>()) {
            nglTextureAnimFrame = params.Get<nglTextureFrameParam>()->field_0;
        } else {
            nglTextureAnimFrame = nglCurScene->IFLFrame;
        }

        g_renderState().setCullingMode(D3DCULL_NONE);

        g_renderState().setBlending(this->field_14->m_blend_mode, this->field_14->field_2C, 0);
        if (EnableShader) {
            IDirect3DDevice9_SetVertexShaderConstantF(g_Direct3DDevice, 0, &this->field_C->WorldToLocal[0][0], 4);

            nglSetVertexDeclarationAndShader(&dword_970AD0());
        } else {
            IDirect3DDevice9_SetTransform(g_Direct3DDevice,
                                          static_cast<D3DTRANSFORMSTATETYPE>(256),
                                          (const D3DMATRIX *)&this->field_C->LocalToWorld);
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[14]);
        }

        static Var<int> dword_956D30{0x00956D30};

        if (dword_956D30() == 1) {
            g_renderState().setCullingMode(D3DCULL_CW);
        } else if (dword_956D30() == 2) {
            g_renderState().setCullingMode(D3DCULL_CCW);
        }

        nglDxSetTexture(0, this->field_14->m_texture, 2u, 3);
        uint32_t v2 = this->field_14->field_28;
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, ((v2 & 0x40) | 0x20u) >> 5);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, ((v2 & 0x80) | 0x40u) >> 6);

        if (EnableShader) {
            SetPixelShader(&PCUV_PShader());
        } else {
            nglSetTextureStageState(0, D3DTSS_COLOROP, 4u);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_COLORARG2, 0);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
            nglSetTextureStageState(1u, D3DTSS_COLOROP, 1u);
            nglSetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
        }

        if (g_distance_clipping_enabled && nglCurScene->field_3BA && !sub_581C30()) {
            g_renderState().setFogEnable(false);
        }

        nglSetStreamSourceAndDrawPrimitive(this->field_10);

        if (g_distance_clipping_enabled && nglCurScene->field_3BA) {
            g_renderState().setFogEnable(true);
        }
    }
}

void us_pcuv_patch()
{
    {
        FUNC_ADDRESS(address, &PCUV_ShaderNode::Render);
        set_vfunc(0x00871BF8, address);
    }
}
