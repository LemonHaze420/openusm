#include "ngl_dx_tex_load.h"

#include "func_wrapper.h"
#include "ngl.h"
#include "trace.h"
#include "variable.h"

#include <windows.h>
#include <d3d9.h>
#include <d3dx9tex.h>

#include <cassert>

static Var<int> SaveCount{0x0093BF2C};

void nglSaveTexture(nglTexture *Tex, const char *a2)
{
    TRACE("nglSaveTexture()", a2);

    assert(Tex != nullptr && "Cannot save an NULL texture !");

    assert(*Tex->DXSurfaces != nullptr && "Cannot save the texture, it has no surface ! (cube texture ?)");

    char Dest[1024]{};
    if (a2 != nullptr) {
        sprintf(Dest, "%s.png", a2);
    } else {
        int v2 = SaveCount()++;
        sprintf(Dest, "savetex%4.4d.png", v2);
    }

    D3DXSaveTextureToFile(Dest, D3DXIFF_PNG, bit_cast<IDirect3DBaseTexture9 *>(Tex->DXTexture), nullptr);
}
