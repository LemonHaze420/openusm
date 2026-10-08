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
        const float origin_x = a3;
        const float space_width = Font->GlyphInfo[32 - Font->Header.FirstGlyph].CellWidth;
        float x = a3;
        float y = a4;
        float scale_x = a5;
        float scale_y = a6;
        float line_scale = scale_y;
        auto *text = reinterpret_cast<char *>(a8);
        int glyph_count = 0;
        a9 = 0;
        while (*text != '\0') {
            const auto character = static_cast<unsigned char>(*text);
            switch (character) {
            case 1: {
                const auto color = static_cast<uint32_t>(std::strtoul(text + 2, &text, 16));
                Color = (color >> 8) | (color << 24);
                ++text;
                break;
            }
            case 2:
                scale_x = scale_y = static_cast<float>(std::strtod(text + 2, &text));
                ++text;
                line_scale = std::max(line_scale, scale_y);
                break;
            case 3:
                scale_x = static_cast<float>(std::strtod(text + 2, &text));
                scale_y = static_cast<float>(std::strtod(text + 1, &text));
                ++text;
                line_scale = std::max(line_scale, scale_y);
                break;
            case 9:
                x += space_width * scale_x * 6.0f;
                ++text;
                break;
            case 10:
                x = origin_x;
                y += Font->Header.CellHeight * line_scale;
                line_scale = scale_y;
                ++text;
                break;
            case 32:
                x += space_width * scale_x;
                ++text;
                break;
            default: {
                auto *section = static_cast<nglStringSection *>(nglListAlloc(sizeof(nglStringSection), 16));
                *section = {};
                section->field_4 = text;
                section->field_10[0] = x;
                section->field_10[1] = y;
                section->field_10[2] = scale_x;
                section->field_10[3] = scale_y;
                section->m_color = Color;
                a2->field_0 = section;
                a2 = section;
                ++a9;
                while (*text != '\0' && *text != 1 && *text != 2 && *text != 3 && *text != 9 && *text != 10 &&
                       *text != 32) {
                    auto glyph = static_cast<unsigned char>(*text++);
                    if (glyph < Font->Header.FirstGlyph || glyph >= Font->Header.FirstGlyph + Font->Header.NumGlyphs) {
                        glyph = 32;
                    }
                    x += Font->GlyphInfo[glyph - Font->Header.FirstGlyph].CellWidth * scale_x;
                    ++section->field_C;
                }
                section->field_8 = static_cast<int>(text - section->field_4);
                glyph_count += section->field_C;
                break;
            }
            }
        }
        a2->field_0 = nullptr;
        return glyph_count;
    } else {
        return static_cast<int>(CDECL_CALL(0x00779570, Font, a2, a3, a4, a5, a6, Color, a8, &a9));
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

                const float z = static_cast<float>(sub_77E820(this->field_1C));
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
                        const float scale_x = static_cast<float>(s_d3dpresent_params.BackBufferWidth) / 640.0f;
                        const float scale_y = static_cast<float>(s_d3dpresent_params.BackBufferHeight) / 480.0f;
                        const float x1 = v21[0] * scale_x;
                        const float y1 = v21[1] * scale_y;
                        const float x2 = v23[0] * scale_x;
                        const float y2 = v23[1] * scale_y;
                        Vertex vertices[4]{
                            {x1, y1, z, 1.0f, v8, a5[0], a5[1]},
                            {x2, y1, z, 1.0f, v8, v31[0], a5[1]},
                            {x1, y2, z, 1.0f, v8, a5[0], v31[1]},
                            {x2, y2, z, 1.0f, v8, v31[0], v31[1]},
                        };

                        IDirect3DDevice9_SetVertexShader(g_Direct3DDevice, nullptr);
                        IDirect3DDevice9_SetPixelShader(g_Direct3DDevice, nullptr);
                        IDirect3DDevice9_SetFVF(g_Direct3DDevice, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
                        nglSetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
                        nglSetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
                        nglSetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                        nglSetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
                        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
                        nglSetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
                        IDirect3DDevice9_DrawPrimitiveUP(
                            g_Direct3DDevice, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
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
