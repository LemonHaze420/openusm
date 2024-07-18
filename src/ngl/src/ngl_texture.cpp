#include "ngl_texture.h"

#include "ngl.h"
#include "ngl_scene.h"
#include "ngl_dx_state.h"
#include "ngl_dx_texture.h"
#include "ngl_dx_shader.h"
#include "variables.h"

#include <cassert>

void nglGenMipmaps(nglTexture *Tex)
{
    assert(Tex->m_format & NGLTEX_RENDER_TARGET && "NGL: Cannot generate mipmap for non-render-target texture.");

    assert(tlIsPow2(Tex->m_width) && tlIsPow2(Tex->m_height) && "NGL: Cannot generate mipmaps for a non power of 2 texture !");

    assert(Tex->m_format & NGLTEX_SWIZZLED && "NGL: Linear textures don't support mipmaps.");

    if ( Tex->m_numLevel > 1 && (Tex->m_format & NGLTEX_RENDER_TARGET) != 0 )
    {
        g_renderState().setCullingMode(D3DCULL_NONE);

        g_renderState().setBlending(NGLBM_OPAQUE, 0, 0);
        nglDxSetTexture(0, Tex, 2u, 3);
        nglSetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        nglSetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        if ( EnableShader )
        {
            nglSetVertexDeclarationAndShader(&stru_975788());
            SetPixelShader(&dword_9757DC());
        }
        else
        {
            nglSetTextureStageState(0, D3DTSS_COLOROP, 2u);
            nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
            nglSetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
            nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);
            nglSetTextureStageState(1u, D3DTSS_COLOROP, 1u);
            nglSetTextureStageState(1u, D3DTSS_ALPHAOP, 1u);
            IDirect3DDevice9_SetTransform(
                g_Direct3DDevice,
                static_cast<D3DTRANSFORMSTATETYPE>(256),
                bit_cast<const D3DMATRIX *>(&nglCurScene()->field_24C));
            IDirect3DDevice9_SetVertexDeclaration(g_Direct3DDevice, dword_9738E0()[25]);
        }

        float v2[20] {};
        v2[8] = stru_946840[0];
        v2[13] = 0.0;
        v2[18] = stru_946840[0];
        v2[9] = 0.0;
        v2[14] = stru_946840[1];
        memset(&v2[2], 0, 12);
        v2[19] = stru_946840[1];
        v2[7] = 0.0;
        v2[12] = 0.0;
        v2[17] = 0.0;
        v2[0] = -1.0;
        v2[1] = 1.0;
        v2[5] = 1.0;
        v2[6] = 1.0;
        v2[10] = -1.0;
        v2[11] = -1.0;
        v2[15] = 1.0;
        v2[16] = -1.0;
        nglSetSamplerState(0, D3DSAMP_MIPFILTER, 0);
        if ( Tex->m_numLevel > 1 )
        {
            for ( uint32_t i = 1; i < Tex->m_numLevel; ++i)
            {
                SetRenderTarget(Tex, nullptr, i, 6);
                IDirect3DDevice9_DrawPrimitiveUP(g_Direct3DDevice, D3DPT_TRIANGLESTRIP, 2, v2, 20);
            }
        }

        g_renderTextureState().clear();
    }
}
