#include "ngl_mesh.h"

#include "common.h"
#include "debug_render.h"
#include "func_wrapper.h"
#include "matrix4x3.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "nglshader.h"
#include "oldmath_po.h"
#include "trace.h"
#include "us_pcuv_shader.h"
#include "utility.h"
#include "vector3d.h"
#include "vtbl.h"

#include <ngl_dx_scene.h>

#include <cassert>

VALIDATE_SIZE(nglMeshParams, 0x20);

VALIDATE_SIZE(nglMesh, 0x40);

void nglMeshInit()
{
    TRACE("nglMeshInit");

    CDECL_CALL(0x0076F420);
}

vector4d sub_76EE90(const vector4d &a2, const vector4d &a3)
{
    auto v7 = a3[2];

    float w = (a2[3] <= v7 ? v7 : a2[3]);
    float z = (a2[2] <= v7 ? v7 : a2[2]);
    float y = (a2[1] <= v7 ? v7 : a2[1]);
    float x = (a2[0] <= v7 ? v7 : a2[0]);

    vector4d result {x, y, z, w};
    return result;
}

math::MatClass<4, 3> *nglListAddMesh_GetScaledMatrix(const math::MatClass<4, 3> &a1,
                                                     nglMeshParams *a2,
                                                     float *a3)
{
    TRACE("nglListAddMesh_GetScaledMatrix");

    if constexpr (1)
    {
        auto v3 = a2->Scale[0];
        auto v4 = a2->Scale[1];
        auto v5 = a2->Scale[2];
        auto v6 = a2->Scale[3];

        vector4d v11;
        v11[0] = std::abs(v3);
        v11[1] = std::abs(v4);
        v11[2] = std::abs(v5);
        v11[3] = std::abs(v6);
        auto v12 = sub_76EE90(v11, v11);

        float v7;
        if (v12[1] <= v11[0]) {
            v7 = v11[0];
        } else {
            v7 = v12[1];
        }

        Var<BOOL> dword_9728F8{0x009728F8};
        bool v8 = dword_9728F8();
        *a3 = v7;
        if (v8) {
            dword_9728F8() |= 1u;
        }

        v12[2] = v5;
        v12[0] = v3;
        v12[1] = v4;
        v12[3] = v6;

        struct {
            vector4d *field_0;
            const math::MatClass<4, 3> *field_4;
        } v10 = {&v12, &a1};

        static matrix4x3 ScaledLocalToWorld;

        auto v13 = sub_771210(&v10);

        std::memcpy(&ScaledLocalToWorld, &v13, sizeof(v13));
        ScaledLocalToWorld[3] = a1[3];

        return (math::MatClass<4, 3> *) &ScaledLocalToWorld;

    } else {
        return (math::MatClass<4, 3> *) CDECL_CALL(0x00770230, &a1, a2, a3);
    }
}

nglMesh *nglListAddMesh_GetLOD(nglMesh *Mesh,
                               unsigned int a2,
                               nglMeshParams *a3,
                               math::VecClass<3, 1> a4)
{
    TRACE("nglListAddMesh_GetLOD");

    if constexpr (1)
    {
        nglMesh *result;
        if ((a2 & 0x80u) == 0)
        {
            math::VecClass<3, 1> v10 = sub_414360(a4, {nglCurScene()->WorldToView});
            auto v7 = Mesh->NLODs - 1;
            if (v7 < 0) {
            LABEL_11:
                result = Mesh;
            } else {
                auto *v8 = Mesh->LODs;
                auto *v9 = (float *) &v8[8 * v7 + 4];
                while (v10[2] <= (double) *v9)
                {
                    --v7;
                    v9 -= 2;
                    if (v7 < 0) {
                        goto LABEL_11;
                    }
                }

                result = *(nglMesh **) (v8 + 8 * v7);
            }
        } else {
            auto v4 = a3->field_C;
            if (v4) {
                auto v6 = Mesh->NLODs;
                if (v4 - 1 <= v6)
                    result = *(nglMesh **) &Mesh->LODs[8 * v4 - 8];
                else
                    result = *(nglMesh **) &Mesh->LODs[8 * v6 - 8];
            } else {
                result = Mesh;
            }
        }
        return result;

    } else {
        return (nglMesh *) CDECL_CALL(0x0, Mesh, a2, a3, a4);
    }
}

bool nglIsSphereVisible(math::VecClass<3, 1> a1, Float radius)
{
    for (auto i = 0u; i < NGLCLIP_MAX; ++i) {
        auto &v = nglCurScene()->ClipPlanes[i];

        if ( a1[0] * v[0] + a1[1] * v[1] + a1[2] * v[2] - v[3] + radius < 0.0f ) {
            return false;
        }
    }

    return true;
}

int nglListAddMesh_GetClipResult(math::VecClass<3, 1> a1, Float radius, int a6)
{
    if constexpr (1)
    {
        if ((a6 & 0x40) != 0 || nglIsSphereVisible(a1, radius)) {
            return 0;
        }

        return -1;

    } else {
        return CDECL_CALL(0x0076F3E0, a1, radius, a6);
    }
}

void nglListAddMesh(nglMesh *Mesh,
                    const math::MatClass<4, 3> &LocalToWorld,
                    nglMeshParams *a3,
                    nglParamSet<nglShaderParamSet_Pool> *a4)
{
    TRACE("nglListAddMesh");

    if (a3 != nullptr) {
        sp_log("%f", a3->Scale[3]);
    }

    if (a4 != nullptr)
    {
        if (a4->IsSetParam<nglTintParam>()) {
            sp_log("color = %f", bit_cast<color *>(a4->Get<nglTintParam>()->field_0)->a );
        }
    }

    if constexpr (0)
    {
        if (Mesh != nullptr)
        {
            assert(((Mesh->Flags & NGLMESH_PROCESSED) || (Mesh->Flags & NGLMESH_SCRATCH_MESH)) &&
               "Mesh missing NGLMESH_PROCESSED flag.");

            if ( nglSyncDebug().DisableScratch
                    || (Mesh->Flags & NGLMESH_SCRATCH_MESH) == 0 )
            {
                return;
            }

            if ( nglSyncDebug().DumpSceneFile ) {
                nglDumpMesh(Mesh, LocalToWorld, a3);
            }

            int v20 = (a3 != nullptr ? a3->Flags : 0);

            auto *v5 = &LocalToWorld;
            auto Radius = Mesh->SphereRadius;
            float v15 = 1.0;
            if ((v20 & NGLP_SCALE) != 0) {
                v5 = nglListAddMesh_GetScaledMatrix(LocalToWorld, a3, &v15);
                Radius *= v15;
            }

            math::VecClass<3, 1> v18 = sub_414360(Mesh->SphereCenter, *v5);
            if (nglCurScene()->field_3E4) {
                nglCalculateMatrices(false);
            }

            if ( Mesh->NLODs != 0 ) {
                Mesh = nglListAddMesh_GetLOD(Mesh, v20, a3, v18);
            }

            auto *v6 = v5;
            auto v17 = nglListWorkPos();

            nglMeshNode *meshNode = new nglMeshNode{};
            meshNode->Mesh = Mesh;
            meshNode->LocalToWorld = matrix4x4 {};

            TransformMatrices v14 {v6, &nglCurScene()->WorldToScreen};

            meshNode->WorldToLocal = sub_507130(v14);
            meshNode->field_84 = 0;
            meshNode->field_80 = nullptr;
            meshNode->field_94 = v15;
            if (a3 != nullptr)
            {
                if (v20 >= 0)
                {
                    meshNode->Params = new nglMeshParams {};
                    std::memcpy(meshNode->Params, a3, sizeof(nglMeshParams));
                } else {
                    meshNode->Params = a3;
                }

            }
            else
            {
                static Var<nglMeshParams> nglEmptyMeshParams{0x00972820};
                meshNode->Params = &nglEmptyMeshParams();
            }

            if (a4 != nullptr) {
                meshNode->field_8C = *a4;
            } else {
                meshNode->field_8C = {static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(0)};
            }

            if (nglListAddMesh_GetClipResult(v18, Radius, v20) == -1)
            {
                nglListWorkPos() = v17;
            }
            else
            {
                for (auto i = 0u; i < Mesh->NSections; ++i)
                {
                    auto *MeshSection = Mesh->Sections[i].Section;
                    nglPerfInfo().m_num_verts += MeshSection->NVertices;

                    MeshSection->Material->m_shader->AddNode(
                            meshNode,
                            MeshSection,
                            MeshSection->Material);
                }

                nglPerfInfo().m_num_polys += Mesh->DataSize;
                auto *v13 = Mesh->File;
                if (v13 != nullptr) {
                    v13->field_144 = nglFrame();
                }
            }
        }
    } else {
        CDECL_CALL(0x00770360, Mesh, &LocalToWorld, a3, a4);
    }
}

void TentacleListAddNode(nglMesh *Mesh, nglBlendModeType a2, const math::VecClass<3, 1> &a3, Float a5, const math::MatClass<4, 3> &a6)
{
    TRACE("TentacleListAddNode");

    if constexpr (1)
    {
        auto v2 = sub_4150E0(a6);
        auto v1 = sub_414360(a3, v2);
        nglMeshSetSphere(v1, a5);
        nglListAddMesh(Mesh, a6, nullptr, nullptr);
    }
    else
    {
        CDECL_CALL(0x00407F10, Mesh, a2, a3, a5, &a6);
    }
}

void PolytubeListAddNode(nglMesh *Mesh,
                nglBlendModeType a2,
                const math::VecClass<3, 1> &a3,
                Float a4,
                const math::MatClass<4, 3> &a5,
                PCUV_ShaderMaterial *a6,
                nglParamSet<nglShaderParamSet_Pool> *a7)
{
    TRACE("PolytubeListAddNode");

    if constexpr (1)
    {
        auto v2 = sub_4150E0(a5);
        auto v1 = sub_414360(a3, v2);
        nglMeshSetSphere(v1, a4);
        a6->m_blend_mode = a2;
        nglListAddMesh(Mesh, a5, nullptr, a7);
    }
    else
    {
        CDECL_CALL(0x00407E90, Mesh, a2, &a3, a4, &a5, a6, a7);
    }

    if constexpr (0) {
        matrix4x4 mat{};
        mat[0] = {1.0f, 0.0f, 0.0f, 0};
        mat[1] = {0, 1.0, 0, 0};
        mat[2] = {0, 0, 1.0, 0};
        mat[3] = {vector3d{a5[3]}, 1.0};

        //sp_log("%s", mat[3].to_string());

        //sp_log("NSections = %d, NBones = %d", Mesh->NSections, Mesh->NBones);

        nglListAddMesh(s_debug_box, {mat}, nullptr, a7);
    }
}

void render_debug_hemisphere(const vector3d &a2, float scale, color32 a4)
{
    TRACE("render_debug_hemisphere");

    if (scale < 0.0049999999) {
        scale = 0.0049999999;
    }

    nglMeshParams meshParams;
    nglParamSet<nglShaderParamSet_Pool> paramSet{static_cast<nglParamSet<nglShaderParamSet_Pool>::nglParamSetType>(1)};

    paramSet.set_color(a4);
    meshParams.Flags |= 2u;
    meshParams.Scale = math::VecClass<3, 1> {scale, scale, scale};

    matrix4x4 v13 {XVEC, YVEC, ZVEC, a2};

    nglListAddMesh(s_debug_hemisphere, v13, &meshParams, &paramSet);

    v13[1] = -YVEC;
    v13[2] = -ZVEC;

    nglListAddMesh(s_debug_hemisphere, v13, &meshParams, &paramSet);
}

tlFixedString *nglMesh::get_string(nglMesh *Mesh) {
    return Mesh->Name;
}

void nglMesh_patch()
{
    //SET_JUMP(0x00770360, nglListAddMesh);

    SET_JUMP(0x00407F10, TentacleListAddNode);

    {
        SET_JUMP(0x00407E90, PolytubeListAddNode);
    }
}
