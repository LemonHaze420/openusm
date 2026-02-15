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

static Var<IDirect3DPixelShader9 *> dword_970770{0x00970770};

static Var<VShader> stru_970760 {0x00970760};

void sub_411830() {
    int i = 0;

    //static Var<const DWORD *> off_939CD8{0x00939CD8};

    {
        static const char text[] =
            "tex t0\n"
            "mul r0, t0, c0\n";

        nglCreatePShader(&(&dword_970770())[i++], text);
    }

    {
        static const char text[] = "mov r0, c0\n";
        nglCreatePShader(&(&dword_970770())[i++], text);
    }
}

static Var<VShader> stru_970768 {0x00970768};

static Var<IDirect3DPixelShader9 *> dword_970774 {0x00970774};

double calc_outline_thickness(const math::VecClass<3,1> &a1)
{
    double (*func)(const void *) = CAST(func, 0x00406B60);
    return func(&a1);
}

namespace USOutlineShaderSpace
{

template<>
void Outline_ShaderNode<USExteriorMaterial>::Render()
{
    TRACE("Outline_ShaderNode<USExteriorMaterial>::Render");

    static Var<int> dword_956FFC {0x00956FFC};
    if ( dword_956FFC() == 0 ) {
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
        if ( g_renderState().field_A8 != 7 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_COLORWRITEENABLE,
                    7);
            g_renderState().field_A8 = 7;
        }

        if ( g_renderState().field_7C != 4 ) {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_ZFUNC,
                    4);
            g_renderState().field_7C = (D3DCMPFUNC) 4;
        }

        if ( !g_renderState().field_74 ) {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_ZWRITEENABLE,
                    1);
            g_renderState().field_74 = 1;
        }

        if ( g_renderState().m_cullingMode != D3DCULL_CW )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_CULLMODE,
                    2);
            g_renderState().m_cullingMode = D3DCULL_CW;
        }

        g_renderTextureState().setSamplerState(0, 8u, 3u);
        nglDxSetTexture(0, this->field_18, 8u, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
        if ( EnableShader )
        {
            nglSetVertexDeclarationAndShader(&stru_970760());
            IDirect3DDevice9_SetVertexShaderConstantF(
                    g_Direct3DDevice,
                    0,
                    &this->m_meshNode->WorldToLocal[0][0],
                    4);
        }
        else
        {
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice,
                (D3DTRANSFORMSTATETYPE)256,
                bit_cast<D3DMATRIX *>(&this->m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(
                    g_Direct3DDevice, dword_9738E0[9]);
        }

        color v18 {};
        if ( EnableShader )
        {
            SetPixelShader(&dword_970770());
            sub_413F80(&v18, this->field_14, &this->m_meshNode->field_8C, 9u);
            v17[0] = v18.r;
            v17[1] = v18.g;
            v17[2] = v18.b;
            v17[3] = v18.a;
            IDirect3DDevice9_SetPixelShaderConstantF(
                    g_Direct3DDevice,
                    0,
                    &v17[0],
                    1);
        }
        else
        {
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
        if ( g_renderState().field_7C != v5 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_ZFUNC,
                    v5);

            g_renderState().field_7C = v5;
        }

        if ( g_renderState().field_74 != v4 ) {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_ZWRITEENABLE,
                    v4);

            g_renderState().field_74 = v4;
        }

        if ( v2->field_68 && EnableShader )
        {
            math::VecClass<3, 1> v17 {};
            
            v17[0] = this->m_meshNode->LocalToWorld[3][0];
            v17[1] = this->m_meshNode->LocalToWorld[3][1];
            v17[2] = this->m_meshNode->LocalToWorld[3][2];
            v17[3] = this->m_meshNode->LocalToWorld[3][3];
            auto v13 = calc_outline_thickness(v17);
            if ( v2->field_7C > 1.f ) {
                v13 *= v2->field_7C;
            }

            if ( g_renderState().field_A8 != 7 )
            {
                IDirect3DDevice9_SetRenderState(
                        g_Direct3DDevice,
                        D3DRS_COLORWRITEENABLE,
                        7);
                g_renderState().field_A8 = 7;
            }

            if ( g_renderState().m_cullingMode != D3DCULL_CCW )
            {
                IDirect3DDevice9_SetRenderState(
                        g_Direct3DDevice,
                        D3DRS_CULLMODE,
                        3);
                g_renderState().m_cullingMode = D3DCULL_CCW;
            }

            nglSetVertexDeclarationAndShader(&stru_970768());
            IDirect3DDevice9_SetVertexShaderConstantF(
                    g_Direct3DDevice,
                    0,
                    &this->m_meshNode->WorldToLocal[0][0],
                    4);

            {
                float v17[4] {};
                v17[3] = v13;
                IDirect3DDevice9_SetVertexShaderConstantF(
                        g_Direct3DDevice, 8, v17, 1);
            }

            SetPixelShader(&dword_970774());

            {
                float v17[4] {};
                v17[0] = v14;
                v17[1] = v15;
                v17[2] = v16;
                v17[3]= 1.0;
                IDirect3DDevice9_SetPixelShaderConstantF(
                        g_Direct3DDevice, 0, v17, 1);
            }
            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        }
    }
}

vector4d sub_41BA70(nglMaterialBase *a1, int *a2, unsigned int a4)
{
    vector4d result;
    CDECL_CALL(0x0041BA70, &result, a1, a2, a4);
    return result;
}

template<>
void Outline_ShaderNode<USInteriorMaterial>::Render()
{
    static Var<int> dword_957000 {0x00957000};
    if ( !dword_957000() )
    {
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
        if ( g_renderState().field_A8 != 7 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice,
                    D3DRS_COLORWRITEENABLE,
                    7);
            g_renderState().field_A8 = 7;
        }

        if ( g_renderState().field_7C != 4 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice, D3DRS_ZFUNC, 4);
            g_renderState().field_7C = (D3DCMPFUNC) 4;
        }

        if ( !g_renderState().field_74 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice, D3DRS_ZWRITEENABLE, 1);
            g_renderState().field_74 = 1;
        }

        if ( g_renderState().m_cullingMode != D3DCULL_CW )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice, D3DRS_CULLMODE, 2);
            g_renderState().m_cullingMode = D3DCULL_CW;
        }

        g_renderTextureState().setSamplerState(0, 8u, 3u);
        nglDxSetTexture(0, this->field_18, 8u, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

        if ( EnableShader )
        {
            nglSetVertexDeclarationAndShader(&stru_970760());
            IDirect3DDevice9_SetVertexShaderConstantF(
                    g_Direct3DDevice,
                    0,
                    &this->m_meshNode->WorldToLocal[0][0],
                    4);
        }
        else
        {
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice,
                static_cast<D3DTRANSFORMSTATETYPE>(256),
                bit_cast<D3DMATRIX *>(&this->m_meshNode->LocalToWorld));
            IDirect3DDevice9_SetVertexDeclaration(
                    g_Direct3DDevice, dword_9738E0[9]);
        }

        if ( EnableShader )
        {
            SetPixelShader(&dword_970770());
            auto v6 = sub_41BA70(this->field_14, (int *)&this->m_meshNode->field_8C, 9u);
            v17[0] = v6[0];
            v17[1] = v6[1];
            v17[2] = v6[2];
            v17[3] = v6[3];
            IDirect3DDevice9_SetPixelShaderConstantF(
                    g_Direct3DDevice, 0, &v17[0], 1);
        }
        else
        {
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
        if ( g_renderState().field_7C != v5 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice, D3DRS_ZFUNC, v5);
            g_renderState().field_7C = v5;
        }

        if ( g_renderState().field_74 != v4 )
        {
            IDirect3DDevice9_SetRenderState(
                    g_Direct3DDevice, D3DRS_ZWRITEENABLE, v4);
            g_renderState().field_74 = v4;
        }

        auto *v8 = this->field_14;
        if ( v2->field_24 && EnableShader )
        {
            v17[0] = this->m_meshNode->LocalToWorld[3][0];
            v17[1] = this->m_meshNode->LocalToWorld[3][1];
            v17[2] = this->m_meshNode->LocalToWorld[3][2];
            v17[3] = this->m_meshNode->LocalToWorld[3][3];
            auto v13 = calc_outline_thickness(*bit_cast<math::VecClass<3, 1> *>(&v17));
            if ( v8->field_38 > 1.f ) {
                v13 *= v8->field_38;
            }

            if ( g_renderState().field_A8 != 7 ) {
                IDirect3DDevice9_SetRenderState(
                        g_Direct3DDevice, D3DRS_COLORWRITEENABLE, 7);
                g_renderState().field_A8 = 7;
            }

            if ( g_renderState().m_cullingMode != D3DCULL_CCW ) {
                IDirect3DDevice9_SetRenderState(
                        g_Direct3DDevice, D3DRS_CULLMODE, 3);
                g_renderState().m_cullingMode = D3DCULL_CCW;
            }

            nglSetVertexDeclarationAndShader(&stru_970768());
            IDirect3DDevice9_SetVertexShaderConstantF(
                    g_Direct3DDevice,
                    0,
                    &this->m_meshNode->WorldToLocal[0][0],
                    4);

            v17[3] = v13;
            v17 = {};

            IDirect3DDevice9_SetVertexShaderConstantF(
                    g_Direct3DDevice, 8, &v17[0], 1);
            SetPixelShader(&dword_970774());

            v17[0] = v14;
            v17[1] = v15;
            v17[2] = v16;
            v17[3] = 1.0;
            IDirect3DDevice9_SetPixelShaderConstantF(
                    g_Direct3DDevice, 0, &v17[0], 1);
            nglSetStreamSourceAndDrawPrimitive(this->m_meshSection);
        }
    }
}

} // USOutlineShaderSpace

void us_outline_patch() {
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
