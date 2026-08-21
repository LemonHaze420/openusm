#pragma once

#include "float.hpp"
#include "matrix4x4.h"
#include "ngl_math.h"
#include "variable.h"
#include "vector3d.h"

#include "ngl_params.h"

struct nglTexture;
struct nglLightContext;
struct nglRenderNode;

enum nglSceneCallbackType {};

enum {
    NGLCLIP_NEAR = 0,
    NGLCLIP_FAR = 1,
    NGLCLIP_LEFT = 2,
    NGLCLIP_RIGHT = 3,
    NGLCLIP_TOP = 4,
    NGLCLIP_BOTTOM = 5,
    NGLCLIP_MAX = 6
};


struct nglScene {
    int field_0;
    uint32_t ClearStencil;
    int field_8;
    matrix4x4 field_C;
    matrix4x4 field_4C;
    matrix4x4 field_8C;
    matrix4x4 ViewToWorld;
    matrix4x4 ViewToScreen;
    matrix4x4 WorldToView;
    matrix4x4 WorldToScreen;
    matrix4x4 field_1CC;
    matrix4x4 field_20C;
    matrix4x4 field_24C;
    math::VecClass<3, 1> ViewPos;
    vector4d ViewDir;
    vector4d ClipPlanes[NGLCLIP_MAX];
    nglScene *field_30C;
    nglScene *field_310;
    nglScene *field_314;
    nglScene *field_318;
    void (*field_31C)(unsigned int *&, void *);
    void *field_320;
    void (*field_324)(unsigned int *&, void *);
    void *field_328;
    void (*field_32C)(unsigned int *&, void *);
    void *field_330;
    nglTexture *field_334;
    nglTexture *field_338;
    int field_33C;
    nglRenderNode *OpaqueNodes;
    nglRenderNode *TransNodes;
    int OpaqueListCount;
    int TransListCount;
    nglLightContext *field_350;
    int field_354[4];
    float vx1;
    float vy1;
    float vx2;
    float vy2;
    float sx1;
    float sy1;
    float sx2;
    float sy2;
    float field_384[4];
    float field_394;
    float field_398;
    uint32_t ClearFlags;
    float ClearZ;
    color ClearColor;
    uint32_t FBWriteMask;
    bool ZWriteEnable;
    bool ZTestEnable;
    bool field_3BA;

    int empty5[8];
    bool field_3DC;
    int field_3E0;
    bool field_3E4;
    float AspectRatio;
    float HFov;
    float m_nearz;
    float m_farz;
    float AnimTime;
    float field_3FC;
    int IFLFrame;
    nglParamSet<nglSceneParamSet_Pool> field_404;
    bool field_408;
    int empty2[12];
    int field_43C;

    void *operator new(size_t);

    void operator delete(void *, size_t) {}
};

//0x0076C700
extern void nglSetupScene(nglScene *a1, nglSceneParamType a2);

extern void nglSetDefaultSceneParams();

extern void nglSceneDumpStart();

extern Var<nglScene *> nglRootScene;

extern nglScene *&nglCurScene;

extern Var<nglScene *> g_shadow_scene;

extern bool nglIsFBPAL();

//0x0076B820
extern void nglSetPerspectiveMatrix(Float a1, Float nearz, Float farz);

extern void nglSetSceneCallBack(nglSceneCallbackType a1, void (*Fn)(unsigned int *&, void *), void *a3);
