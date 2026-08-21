#include "ngl_scene.h"

#include "common.h"
#include "custom_math.h"
#include "error.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "ngl_lighting.h"
#include "ngl_dx_scene.h"
#include "ngl_dx_core.h"
#include "ngl_params.h"
#include "osassert.h"
#include "trace.h"
#include "variables.h"

#include <cassert>

#include <windows.h>

#include <d3dx9shader.h>

VALIDATE_SIZE(nglScene, 0x440u);
VALIDATE_OFFSET(nglScene, field_1CC, 0x1CC);
VALIDATE_OFFSET(nglScene, field_24C, 0x24C);

#if !STANDALONE_SYSTEM
nglScene *&nglCurScene = var<nglScene *>(0x00971F00);
#else
nglScene *&nglCurScene = []() -> auto & {
    static nglScene *g_nglCurScene{};
    return g_nglCurScene;
}();
#endif

Var<nglScene *> nglRootScene{0x00971F04};

Var<nglScene *> g_shadow_scene{0x00965960};

static Var<float> flt_93BC78 = (0x0093BC78);


void *nglScene::operator new(size_t size)
{
    auto *mem = nglListAlloc(size, 64);
    return mem;
}

bool nglIsFBPAL()
{
    return false;
}

void nglSetPerspectiveMatrix(Float a1, Float nearz, Float farz)
{
    assert(nearz > 0.0f && farz > nearz && "Invalid projection parameters.");

    nglCurScene->field_33C = 1;
    nglCurScene->HFov = a1;
    nglCurScene->m_nearz = nearz;
    nglCurScene->m_farz = farz;

    nglCalculateMatrices(true);
}

matrix4x4 Ortho(float a2, float a3, float Near, float Far)
{
    const float v5 = 1.0f / (Far - Near);

    vector4d a5a{0.0f, 0.0f, -Near * v5, 1.0f};

    vector4d a4a{0.0f, 0.0f, v5, 0.0f};

    vector4d a3a{0.0f, a3, 0.0f, 0.0f};

    vector4d a2a{a2, 0.0f, 0.0f, 0.0f};

    matrix4x4 a1{a2a, a3a, a4a, a5a};
    return a1;
}

matrix4x4 Perspective(float a2, float a3, float Near, float Far)
{
    const float v5 = Far / (Far - Near);

    vector4d a2a{a2, 0.0, 0.0, 0.0};

    vector4d a5a{0.0, 0.0, -Near * v5, 0.0};

    vector4d a4a{0.0, 0.0, v5, 1.0};

    vector4d a3a{0.0f, a3, 0.0f, 0.0f};

    matrix4x4 result{a2a, a3a, a4a, a5a};
    return result;
}

matrix4x4 sub_76A870(matrix4x4 &a1)
{
    matrix4x4 result;
    CDECL_CALL(0x0076A870, &result, &a1);

    return result;
}

matrix4x4 sub_76A760(matrix4x4 &a1)
{
    matrix4x4 result;
    CDECL_CALL(0x0076A760, &result, &a1);

    return result;
}

float *sub_64A650(float *a1, const float *a2)
{
    return (float *)CDECL_CALL(0x0064A650, a1, a2);
}

matrix4x4 sub_77CB90()
{
    matrix4x4 result;
    CDECL_CALL(0x0077CB90, &result);

    return result;
}


matrix4x4 Viewport(float a2, float a3, float a4, float a5)
{
    vector4d a2a, a3a, a4a, a5a;
    a5a[0] = (a2 + a4) * 0.5f;
    a5a[2] = 0.0;
    a5a[3] = 1.0;
    a5a[1] = (a3 + a5) * 0.5f;
    a4a[0] = 0.0;
    a4a[1] = 0.0;
    a4a[2] = 1.0;
    a4a[3] = 0.0;
    a3a[0] = 0.0;
    a3a[2] = 0.0;
    a3a[3] = 0.0;
    a3a[1] = (a3 - a5) * 0.5f;
    memset(&a2a[1], 0, 12);
    a2a[0] = (a4 - a2) * 0.5f;
    matrix4x4 result{a2a, a3a, a4a, a5a};
    return result;
}

vector4d sub_76CBD0(const matrix4x4 &a2, Float a3, Float a4, Float a5, Float a6)
{
    vector4d a2a{};
    a2a[1] = a4;
    a2a[2] = a5;
    a2a[0] = a3;
    a2a[3] = a6;
    auto v6 = a2a * *bit_cast<matrix4x3 *>(&a2);
    auto v7 = v6[0];
    auto v8 = v6[1];
    auto v9 = v6[2];
    a2a[0] = v6[0];
    a2a[1] = v6[1];
    auto v11 = v6[2];
    a2a[2] = v11;
    a2a[3] = v6[3];
    auto v15 = a2a[1] * a2[3][1] + v11 * a2[3][2];
    auto v16 = a2a[0] * a2[3][0];

    vector4d result;
    result[0] = v7;
    result[1] = v8;
    result[2] = v9;
    result[3] = v15 + v16 + a6;
    return result;
}

void nglSetOrthoMatrix(Float nearz, Float farz)
{
    assert(farz > nearz && "Invalid projection parameters.");

    nglCurScene->field_33C = 0;
    nglCurScene->HFov = 0.0;
    nglCurScene->m_nearz = nearz;
    nglCurScene->m_farz = farz;

    nglCalculateMatrices(true);
}

matrix4x4 nglGetMatrix(nglMatrixType a2)
{
    if constexpr (0) {
        auto *v2 = nglCurScene;
        if (v2->field_3E4) {
            nglCalculateMatrices(0);
        }

        matrix4x4 result;
        switch (a2) {
        case NGLMTX_VIEW_TO_WORLD:
            result.sub_76CA50(v2->ViewToWorld);
            break;
        case NGLMTX_VIEW_TO_SCREEN:
            result = v2->ViewToScreen;
            break;
        case NGLMTX_WORLD_TO_VIEW:
            result.sub_76CA50(v2->WorldToView);
            break;
        case NGLMTX_WORLD_TO_SCREEN:
            result = v2->WorldToScreen;
            break;
        default:
            assert(0 && "Invalid matrix ID.");
            break;
        }
        return result;
    } else {
        matrix4x4 result;
        CDECL_CALL(0x0076B930, &result, a2);

        return result;
    }
}


void nglCalculateMatrices(bool a1)
{
    TRACE("nglCalculateMatrices", std::to_string(int(a1)).c_str());

    if constexpr (0) {
        if (a1) {
            nglCurScene->field_3E4 = true;
        } else if (nglCurScene->field_3E4) {
            nglCurScene->field_3E4 = false;

            assert((!nglCurScene->OpaqueListCount && !nglCurScene->TransListCount) &&
                   "Illegal projection change after nodes have been added to the scene.");

            assert((nglCurScene->sx2 - nglCurScene->sx1 > 0 && nglCurScene->sy2 - nglCurScene->sy1 > 0) &&
                   "Degenerate scissor!");

            assert((nglCurScene->vx2 - nglCurScene->vx1 > 0 && nglCurScene->vy2 - nglCurScene->vy1 > 0) &&
                   "Degenerate viewport!");

            vector4d v43[NGLCLIP_MAX]{};

            auto vx1 = nglCurScene->vx1;
            auto vy1 = nglCurScene->vy1;
            auto vx2 = nglCurScene->vx2;
            auto vy2 = nglCurScene->vy2;
            auto v108 = 1.0 / (vx2 - vx1);
            auto v107 = (0.0 - (vx1 + vx2)) * v108;
            v108 = v108 + v108;
            auto v106 = (nglCurScene->sx1 * v108) + v107;
            auto v105 = (nglCurScene->sx2 * v108) + v107;
            auto v104 = 1.0 / (vy2 - vy1);
            auto v103 = (0.0 - (vy1 + vy2)) * v104;
            v104 = v104 + v104;
            auto v102 = (nglCurScene->sy1 * v104) + v103;
            auto v101 = (nglCurScene->sy2 * v104) + v103;
            nglCurScene->field_384[0] = v106;
            nglCurScene->field_384[2] = v105;
            nglCurScene->field_384[1] = v102;
            nglCurScene->field_384[3] = v101;

            if (nglCurScene->field_33C == 1) {
                float v45 = nglCurScene->HFov * (0.5f * PI / 180);
                float v46;
                sub_64A650(&v46, &v45);
                v45 = v46 * nglCurScene->AspectRatio;
                auto v7 = std::abs(nglCurScene->field_384[2]);
                auto v8 = std::abs(nglCurScene->field_384[0]);
                float v36 = v8;
                if (v8 <= v7) {
                    v36 = v7;
                }

                auto v9 = std::abs(nglCurScene->field_384[3]);
                auto v10 = std::abs(nglCurScene->field_384[1]);
                float a4 = v10;
                if (v10 > v9) {
                    v9 = a4;
                }

                float v11 = v36 * v45;
                float v12 = v46 * v9;
                nglCurScene->field_394 = v11;
                nglCurScene->field_398 = v12;
                auto v50 = nglCurScene->vx2 + nglCurScene->vx1;
                auto v13 = v50 * 0.5f;
                auto m_nearz = nglCurScene->m_nearz;
                auto m_farz = nglCurScene->m_farz;
                vx1 = (vx1 - v13) * v36 + v13;
                vx2 = (vx2 - v13) * v36 + v13;

                auto v47 = vy2 + vy1;
                auto v15 = v47 * 0.5f;
                vy1 = (vy1 - v15) * v9 + v15;
                vy2 = (vy2 - v15) * v9 + v15;
                auto a3 = 1.0f / v12;
                auto a2 = 1.0f / v11;

                nglCurScene->field_C = Perspective(a2, a3, m_nearz, m_farz);

                v43[4][0] = 0.0;
                v43[4][1] = 0.0;
                v43[4][2] = 1.0;
                v43[4][3] = nglCurScene->m_nearz;
                auto v18 = v45 * v106;
                v43[5][1] = 0.0;
                v43[5][0] = 0.0;
                v43[5][3] = -nglCurScene->m_farz;
                v43[5][2] = -1.0;
                v43[0][1] = 0.0;
                auto v19 = 1.0f / std::sqrt(v18 * v18 + 1.0f);
                v43[0][0] = v19;
                v43[0][3] = 0.0;
                v43[0][2] = -v19 * v18;

                auto v20 = v45 * v105;
                v43[1][1] = 0.0;
                auto v21 = 1.0f / std::sqrt(v20 * v20 + 1.0f);
                v43[1][0] = -v21;
                v43[1][3] = 0.0;
                auto v34 = v21 * v20;
                v43[1][2] = v34;
                auto v22 = v46 * v102;

                v43[2][0] = 0.0;
                auto v23 = 1.0f / std::sqrt(v22 * v22 + 1.0f);
                v43[2][1] = -v23;
                v43[2][2] = -v23 * v22;
                auto v24 = v46 * v101;
                v43[2][3] = 0.0;

                v43[3][0] = 0.0;
                v43[3][3] = 0.0;
                auto v25 = 1.0f / std::sqrt(v24 * v24 + 1.0f);
                v43[3][1] = v25;
                v43[3][2] = v25 * v24;
            } else {
                auto a2 = 1.0f / nglCurScene->AspectRatio;
                nglCurScene->field_C = Ortho(a2, 1.0, nglCurScene->m_nearz, nglCurScene->m_farz);

                v43[4][0] = 0.0;
                v43[4][1] = 0.0;
                v43[4][2] = 1.0;
                v43[4][3] = nglCurScene->m_nearz;

                v43[5][0] = 0.0;
                v43[5][1] = 0.0;
                v43[5][2] = -1.0;
                v43[5][3] = -nglCurScene->m_farz;

                v43[0][0] = 1.0;
                v43[0][1] = 0.0;
                v43[0][2] = 0.0;
                v43[0][3] = nglCurScene->field_384[0] * nglCurScene->AspectRatio;

                v43[1][0] = -1.0;
                v43[1][1] = 0.0;
                v43[1][2] = 0.0;
                v43[1][3] = -nglCurScene->field_384[2] * nglCurScene->AspectRatio;

                v43[2][0] = 0.0;
                v43[2][1] = -1.0;
                v43[2][2] = 0.0;
                v43[2][3] = nglCurScene->field_384[1];

                v43[3][0] = 0.0;
                v43[3][1] = 1.0;
                v43[3][2] = 0.0;
                v43[3][3] = -nglCurScene->field_384[3];
            }

            nglCurScene->field_4C = Viewport(vx1, vy1, vx2, vy2);

            nglCurScene->field_8C = sub_77CB90();

            {
                struct {
                    void *field_0;
                    void *field_4;
                } v47;

                struct {
                    void *field_0;
                    void *field_4;
                } v36{&nglCurScene->field_C, &nglCurScene->field_4C};
                v47.field_0 = &v36;
                v47.field_4 = &nglCurScene->field_8C;
                nglCurScene->ViewToScreen.sub_76CF20(&v47);
            }

            auto a1a = sub_4150E0(nglCurScene->WorldToView);
            nglCurScene->ViewToWorld = a1a;

            {
                struct {
                    void *field_0;
                    void *field_4;
                } v40{&nglCurScene->WorldToView, &nglCurScene->ViewToScreen};

                nglCurScene->WorldToScreen.sub_415A30(&v40);
            }

            nglCurScene->field_1CC = sub_76A870(a1a);
            if (!EnableShader) {
                D3DXMatrixInverse(bit_cast<D3DXMATRIX *>(&nglCurScene->field_24C),
                                  nullptr,
                                  bit_cast<const D3DXMATRIX *>(&nglCurScene->WorldToScreen));
            }

            for (auto i = 0; i < 6; ++i) {
                matrix4x4 v67;
                v67.sub_76CA50(nglCurScene->ViewToWorld);
                nglCurScene->ClipPlanes[i] = sub_76CBD0(v67, v43[i][0], v43[i][1], v43[i][2], v43[i][3]);
            }

            nglCurScene->ViewPos = nglCurScene->ViewToWorld[3];
            nglCurScene->ViewDir = nglCurScene->ViewToWorld[2];

            {
                auto tmp = sub_76A760(a1a);
                matrix4x4 *v40[2]{&tmp, &nglCurScene->field_8C};
                nglCurScene->field_20C.sub_76CE70(&v40);
            }
        }
    } else {
        CDECL_CALL(0x0076ABA0, a1);
    }
}


void nglSetSceneCallBack(nglSceneCallbackType a1, void (*Fn)(unsigned int *&, void *), void *a3)
{
    if (a1) {
        if (a1 == 1) {
            nglCurScene->field_324 = Fn;
            nglCurScene->field_328 = a3;
        } else if (a1 == 2) {
            nglCurScene->field_32C = Fn;
            nglCurScene->field_330 = a3;
        }

    } else {
        nglCurScene->field_31C = Fn;
        nglCurScene->field_320 = a3;
    }
}

void nglSetClearStencil(uint32_t a1)
{
    nglCurScene->ClearStencil = a1;
}

void nglSetLightContext(nglLightContext *a1)
{
    nglCurScene->field_350 = a1;
}

void nglSetDefaultSceneParams()
{
    TRACE("nglSetDefaultSceneParams");

    if constexpr (1) {
        auto *BackBufferTex = nglGetBackBufferTex();
        nglSetRenderTarget(BackBufferTex);
        float v4 = nglGetScreenHeight() - 1;
        float v3 = nglGetScreenWidth() - 1;
        nglSetViewport(0.0, 0.0, v3, v4);
        nglCurScene->AspectRatio = 1.3333334;
        nglCurScene->field_3E4 = true;
        if (g_distance_clipping_enabled) {
            auto v1 = g_distance_clipping * LARGE_EPSILON * (2000.0 - 100.0) + 100.0;
            if (v1 < 100.0) {
                v1 = 100.0;
            }

            nglSetPerspectiveMatrix(50.0, 0.1, v1);
        } else {
            nglSetPerspectiveMatrix(50.0, 0.1, 10000.0);
        }

        char v5;
        math::MatClass<4, 3> a2;
        a2.sub_415740(&v5);

        static Var<vector4d> stru_8B7964{0x008B7964};
        a2[3] = stru_8B7964();
        nglCurScene->WorldToView = sub_4150E0(a2);
        nglCurScene->field_3E4 = true;
        nglCurScene->ClearFlags = 6u;
        nglCurScene->ClearColor = color{0.0, 0.0, 0.0, 0.0};

        nglCurScene->ClearZ = 1.0;
        nglSetClearStencil(0);
        nglCurScene->FBWriteMask = 15u;
        nglCurScene->ZWriteEnable = true;
        nglCurScene->ZTestEnable = true;
        nglCurScene->field_3DC = false;
        nglCurScene->field_3E0 = 0;
        nglCurScene->AnimTime = 0.0;
        nglSetLightContext(nglDefaultLightContext());
        nglCurScene->field_408 = 0;
        nglCalculateMatrices(1);
    } else {
        CDECL_CALL(0x0076C400);
    }
}

void nglSetupScene(nglScene *a1, nglSceneParamType a2)
{
    TRACE("nglSetupScene", std::to_string(uint32_t(a2)).c_str());

    if constexpr (0) {
        nglScene *v2 = nglCurScene;
        nglCurScene = a1;

        switch (a2) {
        case 0: {
            a1 = new nglScene{};

            nglSetDefaultSceneParams();
        } break;
        case 1:
            a1 = v2;
            break;
        case 2:
            a1 = nglRootScene();
            break;
        default:
            break;
        }

#if 0
        a1->field_404 = {1};
#else
        auto *v5 = bit_cast<nglParam *>((unsigned int)(nglListWorkPos() + 7) & 0xFFFFFFF8);
        nglListWorkPos() = bit_cast<uint8_t *>(&v5[1].field_0 + nglSceneParamSet_Pool::NextID());
        v5->field_0 = 0;
        v5->field_4 = 0;
        a1->field_404.field_0 = v5;
#endif

        if (a2 == 1) {
            a1->field_404 = v2->field_404;
        } else if (a2 == 2) {
            a1->field_404 = nglRootScene()->field_404;
        }

        a1->field_30C = v2;
        a1->field_310 = nullptr;
        a1->field_314 = nullptr;
        a1->field_318 = nullptr;
        a1->field_0 = 0;
        a1->field_31C = nullptr;
        a1->field_320 = nullptr;
        a1->field_324 = nullptr;
        a1->field_328 = nullptr;
        a1->field_32C = nullptr;
        a1->field_330 = nullptr;
        if (equal(nglCurScene->AnimTime, 0.0f)) {
            float v7 = (nglIsFBPAL() ? 20.0f : 16.666666f);

            nglCurScene->field_3FC = static_cast<float>(nglFrameVBlankCount()) * v7 * 0.001f;
        } else {
            nglCurScene->field_3FC = nglCurScene->AnimTime;
        }

        auto a2a = nglIFLSpeed * nglCurScene->field_3FC;

        nglCurScene->IFLFrame = static_cast<int>(a2a);
        a1->OpaqueNodes = nullptr;
        a1->TransNodes = nullptr;
        a1->OpaqueListCount = 0;
        a1->TransListCount = 0;
    } else {
        CDECL_CALL(0x0076C700, a1, a2);
    }
}

void nglSceneDumpStart()
{
    if (nglSyncDebug().DumpSceneFile) {
        auto nglHostOpen = [](const char *a1) -> HANDLE {
            CHAR FileName[512];

            strcpy(FileName, a1);
            auto v2 = CreateFileA(FileName, GENERIC_WRITE, 0, nullptr, 2u, 0x8000080u, nullptr);
            if (v2 != (HANDLE)-1) {
                return v2;
            }

            error("cannot open \"%s\" for writing !\n", FileName);

            return nullptr;
        };

        h_sceneDump() = nglHostOpen("scenedump.scene");
        nglHostPrintf(h_sceneDump(), "//\n");
        nglHostPrintf(h_sceneDump(), "// Midnight scene file dump.\n");
        nglHostPrintf(h_sceneDump(), "//\n");
        nglHostPrintf(h_sceneDump(), "\n");
        nglHostPrintf(h_sceneDump(), "\n");
    }
}
