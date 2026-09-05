#include "ngl_dx_font.h"

#include "common.h"
#include "func_wrapper.h"
#include <ngl_dx_shader.h>
#include <ngl_dx_state.h>
#include <ngl_dx_texture.h>
#include "ngl_font.h"
#include "ngl_scene.h"
#include "trace.h"
#include "variables.h"

struct nglStringSection {
    nglStringSection *field_0;
    char *field_4;
    int field_8;
    int field_C;
    float field_10[4];
    uint32_t m_color;
};
VALIDATE_SIZE(nglStringSection, 0x24);

int BuildStringList(nglFont *Font, nglStringSection *a2, Float a3, Float a4, Float a5, Float a6, uint32_t Color,
                    unsigned char *a8, uint32_t &a9)
{
    if constexpr (STANDALONE_SYSTEM) {
        auto *section = static_cast<nglStringSection *>(
            nglListAlloc(sizeof(nglStringSection), alignof(nglStringSection)));
        *section = {};
        section->field_4 = reinterpret_cast<char *>(a8);
        section->field_8 = static_cast<int>(
            std::strlen(reinterpret_cast<const char *>(a8)));
        section->field_10[0] = a3;
        section->field_10[1] = a4;
        section->field_10[2] = a5;
        section->field_10[3] = a6;
        section->m_color = Color;
        a2->field_0 = section;
        a9 = static_cast<uint32_t>(section->field_8);
        return section->field_8;
    } else {
        return static_cast<int>(
            CDECL_CALL(0x00779570, Font, a2, a3, a4, a5, a6, Color, a8, &a9));
    }
}

void nglStringNode::Render()
{
    TRACE("nglStringNode::Render");

    if (nglSyncDebug().DisableFonts) {
        return;
    }

    if constexpr (STANDALONE_SYSTEM) {
        if (this->field_C != nullptr) {
            nglFont *v2 = this->field_10;
            auto v3 = v2->field_40;
            if (v2->field_24 != nullptr) {
                auto perf_counter = query_perf_counter();

                g_renderState().setCullingMode(D3DCULL_NONE);
                g_renderState().setBlending(v2->m_blend_mode, this->field_10->field_48, 128);

                if ((v3 & 0x40) != 0) {
                    nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
                } else {
                    nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
                }

                if ((v3 & 0x80u) == 0) {
                    nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
                } else {
                    nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
                }

                nglDxSetTexture(0, this->field_10->field_24, v3, 3);

                if (EnableShader) {
                    nglSetVertexDeclarationAndShader(&stru_975780());
                } else {
                    IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0[28]);
                    IDirect3DDevice9_SetTransform(g_Direct3DDevice,
                                                  static_cast<D3DTRANSFORMSTATETYPE>(256),
                                                  bit_cast<D3DMATRIX *>(&nglCurScene->field_24C));
                }

                if (EnableShader) {
                    SetPixelShader(&dword_9757A0());
                } else {
                    nglSetTextureStageState(0, D3DTSS_COLOROP, 4u);
                    nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
                    nglSetTextureStageState(0, D3DTSS_COLORARG2, 0);
                    nglSetTextureStageState(0, D3DTSS_ALPHAOP, 4u);
                    nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
                    nglSetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
                    nglSetTextureStageState(1u, D3DTSS_COLOROP, 1u);
                    nglSetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
                    g_renderState().setLighting(0);
                }

                g_renderState().setFogEnable(false);
                auto v4 = this->field_18;
                auto v25 = this->field_14;
                auto a4 = v4;

                static Var<nglStringSection> dword_975690{0x00975690};
                uint32_t a9;
                BuildStringList(this->field_10,
                                &dword_975690(),
                                v25,
                                a4,
                                this->field_20,
                                this->field_24,
                                this->m_color,
                                this->field_C,
                                a9);

                for (auto *i = dword_975690().field_0; i != nullptr; i = i->field_0) {
                    auto v6 = i->field_10[2];
                    auto v7 = i->field_10[3];
                    auto v8 = i->m_color;
                    auto v25 = i->field_10[0];
                    auto v9 = i->field_8;
                    auto a7 = v6;
                    auto v10 = i->field_10[1];
                    auto a8 = v7;
                    auto a4 = v10;

                    while (v9--) {
                        auto v12 = this->field_10;

                        auto v11 = *i->field_4;

                        float v21[2];
                        float v23[2];
                        float a5[2];
                        float v31[2];
                        v12->sub_77E2F0(v11, v21, v23, a5, v31, a7, a8);
                        v21[0] = v21[0] + v25;
                        v23[0] = v23[0] + v21[0];
                        v21[1] = v21[1] + a4;
                        v23[1] = v23[1] + v21[1];
                        v31[0] = v31[0] + a5[0];
                        v31[1] = v31[1] + a5[1];
                        struct Vertex {
                            float x;
                            float y;
                            float z;
                            float rhw;
                            uint32_t color;
                            float u;
                            float v;
                        };
                        const float scale_x =
                            static_cast<float>(s_d3dpresent_params.BackBufferWidth) / 640.0f;
                        const float scale_y =
                            static_cast<float>(s_d3dpresent_params.BackBufferHeight) / 480.0f;
                        const float x1 = v21[0] * scale_x;
                        const float y1 = v21[1] * scale_y;
                        const float x2 = v23[0] * scale_x;
                        const float y2 = v23[1] * scale_y;
                        Vertex vertices[4]{
                            {x1, y1, 0.5f, 1.0f, v8, a5[0], a5[1]},
                            {x2, y1, 0.5f, 1.0f, v8, v31[0], a5[1]},
                            {x1, y2, 0.5f, 1.0f, v8, a5[0], v31[1]},
                            {x2, y2, 0.5f, 1.0f, v8, v31[0], v31[1]},
                        };

                        IDirect3DDevice9_SetVertexShader(g_Direct3DDevice, nullptr);
                        IDirect3DDevice9_SetPixelShader(g_Direct3DDevice, nullptr);
                        IDirect3DDevice9_SetFVF(
                            g_Direct3DDevice,
                            D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
                        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                        nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                        nglSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                        IDirect3DDevice9_DrawPrimitiveUP(
                            g_Direct3DDevice,
                            D3DPT_TRIANGLESTRIP,
                            2,
                            vertices,
                            sizeof(Vertex));
                        double v18 = this->field_10->GetFontCellWidth(v11);
                        if (v18 < 0) {
                            v18 += flt_86F860;
                        }

                        auto v19 = v18 * i->field_10[2];
                        ++i->field_4;
                        v25 += v19;
                    }
                }

                dword_975690().field_0 = nullptr;
                if (g_distance_clipping_enabled && !sub_581C30()) {
                    g_renderState().setFogEnable(true);
                }

                nglPerfInfo().field_50.QuadPart += query_perf_counter().QuadPart - perf_counter.QuadPart;
            }
        }
    } else {
        THISCALL(0x0077E3E0, this);
    }
}
