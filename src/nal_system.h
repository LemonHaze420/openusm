#pragma once

#include "charanimtype/charcomponentbase.h"
#include "fixedstring.h"
#include "float.hpp"
#include "hashstring.h"
#include "nal_anim.h"
#include "nfl_system.h"
#include "tl_system.h"
#include "variable.h"

#include <nal_generic.h>
#include <nalcomp/nal_pose_comp.h>

#include <memory>

extern tlInstanceBank &nalTypeInstanceBank;

extern tlInstanceBank &nalComponentInstanceBank;

extern LARGE_INTEGER &nalPlayerGetPoseTicks;

struct nal_anim_control {
    uint32_t m_vtbl;
};

struct nalVector3 {
    float field_0[3];

    float &operator[](uint32_t idx)
    {
        return this->field_0[idx];
    }

    float operator[](uint32_t idx) const
    {
        return this->field_0[idx];
    }

    nalVector3 operator+(const nalVector3 &v) const
    {
        nalVector3 result;
        result[0] = this->field_0[0] + v[0];
        result[1] = this->field_0[1] + v[1];
        result[2] = this->field_0[2] + v[2];

        return result;
    }

    nalVector3 operator-(const nalVector3 &v) const
    {
        nalVector3 result;
        result[0] = this->field_0[0] - v[0];
        result[1] = this->field_0[1] - v[1];
        result[2] = this->field_0[2] - v[2];

        return result;
    }

    nalVector3 operator*(float a1) const
    {
        nalVector3 result;
        result[0] = this->field_0[0] * a1;
        result[1] = this->field_0[1] * a1;
        result[2] = this->field_0[2] * a1;

        return result;
    }
};

struct nalMatrix4x4 {
    float arr[4][4];

    nalMatrix4x4() = default;

    nalMatrix4x4(const nalPositionOrientation &a2);

    auto &operator[](int idx)
    {
        return arr[idx];
    }

    const auto &operator[](int idx) const
    {
        return arr[idx];
    }

    void sub_5FC9C0(const nalPositionOrientation &a2);
};

extern nalMatrix4x4 &stru_9771C0;

extern nalMatrix4x4 sub_5FE000(const nalMatrix4x4 &arg4, const nalMatrix4x4 &arg8);

extern nalMatrix4x4 sub_5F2FD0(Float a2, const float *a3);

extern float sub_5F4960(const nalMatrix4x4 &a2, bool a3);

struct nalPositionOrientation {
    float field_0[4];
    nalVector3 field_10;

    nalPositionOrientation() = default;

    nalPositionOrientation(nalVector3, const float *);

    static inline nalPositionOrientation &Identity = var<nalPositionOrientation>(0x00977180);
};

struct nalBasePose {
    nalComp::nalCompSkeleton *field_0;

    auto GetSkeleton() const
    {
        return this->field_0;
    }
};

struct nalBaseSkeleton;

struct nalAnimFile {
    uint32_t field_0;
    uint32_t field_4;
    int field_8;
    int num_skeletons;
    tlFixedString field_10;
    int field_30;
    nalAnimClass<nalAnyPose> *field_34;
    tlFileBuf field_38;
    int field_44;
    tlFixedString field_48;

    static tlFixedString *get_string(nalAnimFile *a1)
    {
        return &a1->field_10;
    }
};

struct nalBaseSkeleton;

//0x0078DC60
extern nalBaseSkeleton *nalGetSkeleton(const tlFixedString &a1);

struct nalComponentBase {
    int m_vtbl;

    //virtual
    void Process(const nalGeneric::nalComponentInfo *a1, void *&a2, void *&a3);  // = 0;
};

struct nalComponentPOBase : nalComponentBase {
    /* virtual */
    int *_GetType()
    {
        return &TypeID;
    }

    static inline int TypeID{0};
};

struct nalComponentFloat1Base : nalComponentBase {
    /* virtual */ int *_GetType();

    static inline int TypeID{0};
};

struct nalComponentFloat3Base : nalComponentBase {
    /* virtual */ int *_GetType();

    static inline int TypeID{0};
};

struct nalComponentQuatBase : nalComponentBase {
    /* virtual */ int *_GetType();

    static inline int TypeID{0};
};

struct nalComponentU8Base : nalComponentBase {
    /* virtual */ int *_GetType();

    static inline int TypeID{0};
};

struct nalComponentStringBase : nalComponentBase {
    char *GetType();

    static inline char &TypeID = var<char>(0x00959560);
};

struct nalComponentInitList;

template <typename T>
T nalSkeletonPtrCast(T a1)
{
    if (a1 != nullptr || *a1 != nalGeneric::nalGenericSkeleton::vtbl_ptr) {
        return nullptr;
    }

    return a1;
}


struct nalHeap;
struct nalSceneAnim;

//0x00783CF0
extern void nalInit(nalHeap *a1);

//0x00783C60
extern void nalExit();

//0x0078D9B0
extern void nalReleaseSceneAnimInternal(nalSceneAnim *a1);

//0x0078D8D0
extern bool nalLoadSceneAnimInternal(nalSceneAnim *a1);

//0x0078D540
extern bool nalLoadAnimFileInternal(nalAnimFile *anim_file);

extern void nalSetSkeletonDirectory(tlResourceDirectory<nalBaseSkeleton, tlFixedString> *a1);

//0x0050EAB0
extern tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString> *nalGetSkeletonDirectory();

extern void nalSetAnimFileDirectory(tlResourceDirectory<nalAnimFile, tlFixedString> *a1);

//0x0050EA50
extern tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString> *nalGetAnimFileDirectory();

struct nalAnyPose;

template <typename>
struct nalAnimClass;

extern void nalSetAnimDirectory(tlResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *a1);

//0x0050EA70
extern tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *nalGetAnimDirectory();

extern void nalSetSceneAnimDirectory(tlResourceDirectory<nalSceneAnim, tlFixedString> *a1);

//0x0050EA90
extern tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString> *nalGetSceneAnimDirectory();

extern char (&nalSkeletonPath)[255];

extern char (&nalAnimPath)[255];

struct actor;
struct string_hash;

namespace als {
struct als_meta_anim_table_shared;
}

//0x0049B910
extern void *get_anim_by_hash(const string_hash &a1, const als::als_meta_anim_table_shared *a2, actor *a3);

struct IKSkelData {
    float field_0;
    float field_4;
    float field_8;
    float field_C;
    float field_10;
};

extern void DecomposeIKSpin(nalMatrix4x4 &a1, nalMatrix4x4 &a2, const nalMatrix4x4 &a3, const nalVector3 &a4,
                            const nalMatrix4x4 &a5, const IKSkelData &a6,
                            nalVector3 (*a7)(const nalMatrix4x4 &, const nalMatrix4x4 &, nalVector3), Float a8);

extern nalVector3 LegHeuristic(const nalMatrix4x4 &a2, const nalMatrix4x4 &a3, nalVector3 a4);

extern nalMatrix4x4 sub_5F4170(Float a2, Float a3, const nalVector3 &a4);

extern void sub_5F42D0(nalMatrix4x4 &a1, nalMatrix4x4 &a2, Float a3, const nalVector3 &a4, const nalVector3 &a5,
                       bool a6);

extern void nalStreamInstance_patch();
