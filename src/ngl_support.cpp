#include "ngl_support.h"

#include "func_wrapper.h"
#include "ngl.h"
#include "ngl_dx_scene.h"
#include "ngl_scene.h"
#include "nglshader.h"
#include "oldmath_po.h"
#include "trace.h"
#include "vtbl.h"

#include <cmath>


void FastListAddMesh(nglMesh *Mesh,
                     const math::MatClass<4, 3> &LocalToWorld,
                     nglMeshParams *MeshParams,
                     nglParamSet<nglShaderParamSet_Pool> *ShaderParams)
{
    TRACE("FastListAddMesh");

    if (ShaderParams != nullptr)
    {
        if (ShaderParams->IsSetParam<nglTintParam>()) {

            auto *col = bit_cast<color *>(ShaderParams->Get<nglTintParam>()->field_0);
            sp_log("color = %f %f %f %f", col->r, col->g, col->b, col->a);
        }
    }

    if constexpr (1)
    {
        assert(Mesh != nullptr && "NULL mesh passed to FastListAddMesh.\n");

        assert(((Mesh->Flags & NGLMESH_PROCESSED) ||
               (Mesh->Flags & NGLMESH_SCRATCH_MESH)) && "Mesh missing NGLMESH_PROCESSED flag.");

        assert(std::abs(AbsSquared(LocalToWorld.GetX()) - 1.0f) +
                       std::abs(AbsSquared(LocalToWorld.GetY()) - 1.0f) +
                       std::abs(AbsSquared(LocalToWorld.GetZ()) - 1.0f) <
                   0.01f &&
               "Invalid scale detected in local to world transform.  If scaling is desired, use "
               "MeshParams.\n");

        if (0) //(nglSyncDebug().field_12) {
        {
            nglDumpMesh(Mesh, LocalToWorld, MeshParams);
        }

        assert(MeshParams != nullptr && "NULL MeshParams in FastListAddMesh.\n");

        if (Mesh->NLODs != 0)
        {
            math::VecClass<3, 1> v5 = ( (MeshParams->Flags & 1) != 0
                                            ? Mesh->SphereCenter
                                            : sub_414360(Mesh->SphereCenter, LocalToWorld)
                                        );

            math::VecClass<3, 1> a2a = v5;

            auto v18 = sub_414360(a2a, nglCurScene()->WorldToView);

#if 0
            auto v9 = Mesh->NLODs - 1;
            if (v9 >= 0) {
                auto *v10 = Mesh->LODs;
                auto *v11 = (float *) &v10[v9].field_4;
                while (v18.field_0[2] <= (double) *v11) {
                    --v9;
                    v11 -= 2;
                    if (v9 < 0) {
                        goto LABEL_11;
                    }
                }

                Mesh = v10[v9].field_0;
            }
#else
            auto GetLOD = [](nglMesh *a1, float a2) -> nglMesh *
            {
                for ( int i = a1->NLODs - 1; i >= 0; --i )
                {
                    if ( a2 > a1->LODs[i].field_4 ) {
                        return a1->LODs[i].field_0;
                    }
                }

                return a1;
            };
            Mesh = GetLOD(Mesh, v18[2]);
#endif
        }

        auto *meshNode = new nglMeshNode {};
        meshNode->Mesh = Mesh;
        meshNode->LocalToWorld = LocalToWorld;

        TransformMatrices a2a;
        a2a.m_rel_po = CAST(a2a.m_rel_po, &LocalToWorld);
        a2a.m_abs_po = CAST(a2a.m_abs_po, &nglCurScene()->WorldToScreen);

        meshNode->WorldToLocal = sub_507130(a2a);

        meshNode->field_84 = 0;
        meshNode->field_80 = nullptr;
        meshNode->field_94 = 1.0;

        assert(!(MeshParams->Flags & NGLP_SCALE) && "No scale allowed in FastListAddMesh.\n");

        //assert((MeshParams->Flags & NGLP_REFERENCED) && "MeshParams must be referenced in FastListAddMesh.\n");

        assert((MeshParams->Flags & NGLP_NO_CULLING) &&
               "Mesh must be pre-culled for FastListAddMesh.\n");

        assert(!(MeshParams->Flags & NGLP_FORCE_LOD) &&
               "Force LOD not supported by FastListAddMesh.\n");

        meshNode->Params = MeshParams;

        assert(ShaderParams != nullptr && "NULL ShaderParams in FastListAddMesh.\n");

        meshNode->field_8C = *ShaderParams;

        for (auto i = 0u; i < Mesh->NSections; ++i)
        {
            auto *MeshSection = Mesh->Sections[i].Section;

            nglPerfInfo().m_num_verts += MeshSection->NVertices;

            nglMaterialBase *v15 = sub_8EA2E0(&meshNode->field_8C, MeshSection->Material);

            MeshSection->Material->m_shader->AddNode(meshNode, MeshSection, v15);
        }

        nglPerfInfo().m_num_polys += Mesh->DataSize;

        if (Mesh->File != nullptr) {
            Mesh->File->field_144 = nglFrame();
        }
    } else {
        CDECL_CALL(0x00507690, Mesh, LocalToWorld, MeshParams, ShaderParams);
    }
}
