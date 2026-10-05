#pragma once

#include <cstdint>

#include <windows.h>

#include <d3d9types.h>

struct nglTexture;

//0x007754B0
extern void nglDxSetTexture(uint32_t a1, nglTexture *Tex, uint8_t a3, int a4);

//0x0076DC30
extern void nglSetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value);

//0x0076DC70
extern void nglSetTextureStageState(DWORD a1, D3DTEXTURESTAGESTATETYPE a2, DWORD a3);

extern void nglResetTextureState();
extern void nglSetDepthBias(float bias);

//
extern void nglDxSetTexel8(nglTexture *Tex, int a2, int a3, int a4);

//0x00775240
extern bool nglDxLockTexture(nglTexture *a1, int flags);

//0x00775270
extern HRESULT nglDxUnlockTexture(nglTexture *Tex);
