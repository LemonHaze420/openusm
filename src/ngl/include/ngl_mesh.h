#pragma once

#include "hashstring.h"
#include "ngl_math.h"
#include "ngl_params.h"
#include "variable.h"

struct nglMeshSection;
struct nglMeshFile;

struct nglMeshParams {
    uint32_t Flags;
    int NBones;
    math::MatClass<4, 3> *Bones;
    int field_C;
    math::VecClass<3, 1> Scale;

    nglMeshParams() = default;

    nglMeshParams(uint32_t f) : Flags(f) {}
};

inline constexpr auto NGLMESH_PROCESSED = 0x10000;
inline constexpr auto NGLMESH_SCRATCH_MESH = 0x20000;
inline constexpr auto NGLMESH_TEMP = 0x40000;
inline constexpr auto NGLMESH_STATIC = 0x80000;

struct tlFixedString;

struct nglMesh {
    tlFixedString *Name;
    uint32_t Flags;
    uint32_t NSections;

    struct {
        int field_0;
        nglMeshSection *Section;
    } * Sections;
    int NBones;
    math::MatClass<4, 3> *Bones;
    int NLODs;
    struct Lod {
        nglMesh *field_0;
        float field_4;
    };

    Lod *LODs;
    math::VecClass<3, 1> SphereCenter;
    float SphereRadius;
    nglMeshFile *File;
    nglMesh *NextMesh;
    uint32_t DataSize;

    static tlFixedString *get_string(nglMesh *Mesh);
};

//0x0076F420
extern void nglMeshInit();

//0x00770230
extern math::MatClass<4, 3> *nglListAddMesh_GetScaledMatrix(const math::MatClass<4, 3> &a1,
                                                            nglMeshParams *a2,
                                                            float *a3);

//0x00770190
extern nglMesh *nglListAddMesh_GetLOD(nglMesh *Mesh,
                                      unsigned int a2,
                                      nglMeshParams *a3,
                                      math::VecClass<3, 1> a4);

extern matrix4x4 sub_507130(void *arg4);

//0x00770360
extern void nglListAddMesh(nglMesh *Mesh,
                           const math::MatClass<4, 3> &,
                           nglMeshParams *,
                           nglParamSet<nglShaderParamSet_Pool> *);

extern void render_debug_hemisphere(const vector3d &a2, float scale, color32 a4);

extern void nglMesh_patch();
