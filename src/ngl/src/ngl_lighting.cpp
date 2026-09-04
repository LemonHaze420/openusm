#include "ngl_lighting.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "trace.h"
#include "utility.h"

#include <ngl_dx_scene.h>

VALIDATE_SIZE(nglDirLightInfo, 0x20);

VALIDATE_SIZE(nglPointLightInfo, 0x28);

VALIDATE_SIZE(nglLightNode, 0x30);

VALIDATE_SIZE(nglLightContext, 0x70);

Var<nglLightContext *> nglDefaultLightContext{0x00973B70};

Var<nglLightContext *> nglCurLightContext{0x00973B74};

void nglDumpDirLight(unsigned int LightCat, vector4d Dir, vector4d a4)
{
    nglHostPrintf(h_sceneDump(), "\n");
    nglHostPrintf(h_sceneDump(), "DIRLIGHT\n");
    nglHostPrintf(h_sceneDump(), "  LIGHTCAT 0x%8X\n", LightCat);
    nglHostPrintf(h_sceneDump(), "  DIR %f %f %f\n", Dir[0], Dir[1], Dir[2]);
    nglHostPrintf(h_sceneDump(), "  COLOR %f %f %f %f\n", a4[0], a4[1], a4[2], a4[3]);
    nglHostPrintf(h_sceneDump(), "ENDLIGHT\n");
}

bool nglGetFakePointLight(nglDirLightInfo *a1, nglPointLightInfo *a2,
                          math::VecClass<3, 1, void, void, math::Rep_Std<false>> a3)
{
    return (bool)CDECL_CALL(0x007765C0, a1, a2, a3);
}

nglDirLightInfo *nglGetLightAsDirLight(nglDirLightInfo *a1, nglLightNode *a2,
                                       math::VecClass<3, 1, void, void, math::Rep_Std<false>> a3)
{
    auto Type = a2->Type;
    if (Type) {
        if (Type == 1) {
            return static_cast<nglDirLightInfo *>(a2->Data);
        } else {
            assert(0 && "Unsupported light type.");
            return nullptr;
        }
    } else {
        nglGetFakePointLight(a1, static_cast<nglPointLightInfo *>(a2->Data), a3);
        return a1;
    }
}


void nglListAddLight(nglLightType Type, void *Data, uint32_t LightCat)
{
    if constexpr (0) {
        nglLightNode *v3 = static_cast<nglLightNode *>(nglListAlloc(sizeof(nglLightNode), 16));
        if (v3 != nullptr) {
            v3->Data = Data;
            v3->Type = Type;
            v3->LightCat = LightCat;

            for (int i = 0; i < 8; ++i) {
                if ((LightCat & (1 << (i + NGL_LIGHTCAT_SHIFT))) != 0) {
                    v3->Next[i] = nglCurLightContext()->Head.Next[i];
                    nglCurLightContext()->Head.Next[i] = v3;
                }
            }
        }
    } else {
        CDECL_CALL(0x00775C80, Type, Data, LightCat);
    }
}


void nglListAddPointLight(uint32_t LightCat, math::VecClass<3, 1> a2, Float a6, Float radius, math::VecClass<4, -1> a8)
{
    TRACE("nglListAddPointLight");

    if (nglIsSphereVisible(a2, radius)) {
        auto *Light = static_cast<nglPointLightInfo *>(nglListAlloc(sizeof(nglPointLightInfo), 16));
        if (Light != nullptr) {
            Light->ViewPos = a2;
            Light->Pos[0] = a8[0];
            Light->Pos[1] = a8[1];
            Light->Pos[2] = a8[2];
            Light->Pos[3] = 1.0;
            Light->Near = a6;
            Light->Far = radius;
            nglListAddLight(static_cast<nglLightType>(0), Light, LightCat);
        }
    }
}

void nglListAddDirLight(unsigned int a2, math::VecClass<3, 0, void, math::VecUnit<1>, math::Rep_Std<false>> a3,
                        math::VecClass<4, -1, void, void, math::Rep_Std<false>> a4)
{
    TRACE("nglListAddDirLight");

    auto *Light = static_cast<nglDirLightInfo *>(nglListAlloc(sizeof(nglDirLightInfo), 16));
    if (Light != nullptr) {
        Light->Dir = a3;
        Light->Color[0] = a4[0];
        Light->Color[1] = a4[1];
        Light->Color[2] = a4[2];
        Light->Color[3] = 1.0;
        nglListAddLight(static_cast<nglLightType>(1), Light, a2);
    }

    if (nglSyncDebug().DumpSceneFile) {
        vector4d v8 = *bit_cast<vector4d *>(&a3);
        vector4d v9 = *bit_cast<vector4d *>(&a4);
        nglDumpDirLight(a2, v8, v9);
    }
}

void ngl_lighting_patch()
{
    SET_JUMP(0x00776140, nglListAddPointLight);

    SET_JUMP(0x007760C0, nglListAddDirLight);
}
