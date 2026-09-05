#include "ngl_dx_texture.h"

#include "func_wrapper.h"
#include "variable.h"
#include "variables.h"
#include "trace.h"

#include <ngl.h>

#include <cassert>

void sub_782AC0(int, nglPalette *a2)
{
    if (!g_valid_texture_format) {
        IDirect3DDevice9_SetCurrentTexturePalette(g_Direct3DDevice, a2->m_palette_idx);
    }
}

void nglDxSetTexture(uint32_t a1, nglTexture *Tex, uint8_t a3, int a4)
{
    TRACE("nglDxSetTexture");

    if constexpr (STANDALONE_SYSTEM) {
        assert(Tex != nullptr && "NULL texture pointer.");

        auto *v4 = Tex;
        auto *v5 = Tex;
        if (static_cast<uint8_t>(Tex->m_format) == 16) {
            auto v6 = Tex->m_num_palettes;

            unsigned int v7;
            if (nglTextureAnimFrame < v6) {
                v7 = nglTextureAnimFrame;
            } else {
                v7 = nglTextureAnimFrame % v6;
            }

            v5 = Tex->Frames[v7];
        }

        assert(v5 != nullptr && "encountered invalid IFL data.\n");

        auto *palette = v5->field_48;
        auto *v16 = palette;
        if (static_cast<uint8_t>(v5->m_format) == 17) {
            v5 = (nglTexture *)v5->m_num_palettes;
        }

        auto v9 = a1;
        auto *v10 = v5->DXTexture;
        v5->field_38 = nglFrame();

        if (g_renderTextureState().field_0[a1] != v10) {
            IDirect3DDevice9_SetTexture(g_Direct3DDevice, a1, (IDirect3DBaseTexture9 *)v10);
            g_renderTextureState().field_0[a1] = v5->DXTexture;
        }

        if (palette != nullptr) {
            if (g_valid_texture_format && (Tex->field_34 & 8) != 0 && NGLTEX_GET_FORMAT(v5->m_format) == 7) {
                for (auto v11 = 0u; v11 < v5->m_num_palettes; ++v11) {
                    v5->Frames[v11]->field_34 |= 8u;
                }

                v5->DXTexture->lpVtbl->LockRect(v5->DXTexture, 0, &v5->field_24, nullptr, 0);
                auto *v17 = (char *)v5->field_24.pBits;
                for (int i = 0; i < v5->m_height; ++i) {
                    auto *v14 = (uint32_t *)&v17[i * v5->field_24.Pitch];
                    if (v5->m_width) {
                        auto BYTE2 = [](uint32_t &v) -> uint8_t {
                            auto result = bit_cast<uint8_t *>(&v)[2];
                            return result;
                        };

                        uint32_t v15 = 0;
                        do {
                            v14[v15] =
                                (*(uint32_t *)&v16->m_palette_entries[v5->field_30[i * v5->m_width + v15]] &
                                 0xFF00FF00) |
                                ((uint8_t)*(uint32_t *)&v16->m_palette_entries[v5->field_30[i * v5->m_width + v15]]
                                 << 16) |
                                (uint8_t)BYTE2(
                                    *(uint32_t *)&v16->m_palette_entries[v5->field_30[i * v5->m_width + v15]]);
                            ++v15;
                        } while (v15 < static_cast<uint32_t>(v5->m_width));
                        v4 = Tex;
                        v9 = a1;
                    }
                }

                v5->DXTexture->lpVtbl->UnlockRect(v5->DXTexture, 0);
                palette = v16;
                v4->field_34 &= 0xFFFFFFF7;
            }

            sub_782AC0(v9, palette);
        }

        g_renderTextureState().setSamplerState(v9, a3, a4);
    } else {
        CDECL_CALL(0x007754B0, a1, Tex, a3, a4);
    }
}

void nglSetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
{
    const auto state_index = static_cast<std::size_t>(type) + 14u * sampler;
    static Var<uint32_t[112]> sampler_states{0x00971FF0};
    if (sampler_states()[state_index] != value) {
        IDirect3DDevice9_SetSamplerState(g_Direct3DDevice, sampler, type, value);
        sampler_states()[state_index] = value;
    }
}

void nglSetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
{
    const auto state_index = static_cast<std::size_t>(type) + 33u * stage;
    static Var<DWORD[264]> texture_stage_states{0x00972240};
    if (texture_stage_states()[state_index] != value) {
        IDirect3DDevice9_SetTextureStageState(g_Direct3DDevice, stage, type, value);
        texture_stage_states()[state_index] = value;
    }
}

void nglDxSetTexel8(nglTexture *Tex, int a2, int a3, int a4)
{
    assert(Tex != nullptr && "Texture is NULL!");

#if 0
    assert(NGLTEX_GET_FORMAT(Tex->m_format) == NGLTEX_L8)
            || (NGLTEX_GET_FORMAT(Tex->m_format) == NGLTEX_I8)
            || (NGLTEX_GET_FORMAT(Tex->m_format) == NGLTEX_A8)
            || (NGLTEX_GET_FORMAT(Tex->m_format) == NGLTEX_AL8);
#endif

    auto *pBits = (char *)Tex->field_24.pBits;
    if ((Tex->m_format & 0x200) != 0) {
        *(DWORD *)&pBits[4 * a2 + a3 * Tex->field_24.Pitch] = a4;
    } else {
        *(DWORD *)pBits = a4;
    }
}

static int nglTexLocked{0};


bool nglDxLockTexture(nglTexture *Tex, int flags)
{
    assert(nglTexLocked == 0 && "Texture still locked ! (only one texture at a time can be locked)");
    assert(Tex != nullptr && "nglDxLockTexture: Cannot lock a NULL texture !");

    auto result = (Tex->DXTexture->lpVtbl->LockRect(Tex->DXTexture, 0, &Tex->field_24, nullptr, flags) == 0);
    nglTexLocked = result;

    return result;
}

HRESULT nglDxUnlockTexture(nglTexture *Tex)
{
    assert(nglTexLocked && "Must call nglDxLockTexture before calling nglDxUnlockTexture !");
    assert(Tex != nullptr && "nglDxLockTexture: Cannot lock a NULL texture !");

    auto result = Tex->DXTexture->lpVtbl->UnlockRect(Tex->DXTexture, 0);

    nglTexLocked = 0;

    return result;
}

void ngl_dx_texture_patch() {}
