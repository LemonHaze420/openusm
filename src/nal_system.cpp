#include "nal_system.h"

#include "charanimtype/character_pose_skel.h"
#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "nal_anim.h"
#include <nal_anim_comp.h>
#include "nal_component.h"
#include "nfl_system.h"
#include "osassert.h"
#include "scene_anim.h"
#include "tl_system.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vector4d.h"
#include "vtbl.h"

#include <nal_list.h>
#include <nal_skeleton.h>

#include <cassert>

VALIDATE_OFFSET(nalGeneric::nalGenericSkeleton, field_50, 0x50);

VALIDATE_SIZE(IKSkelData, 0x14);

struct nalHeap {
    std::intptr_t m_vtbl;
    uint32_t field_4;
    int field_8;
};

struct nalAnimCache {
    nalHeap *field_0;
    int field_4;
    int field_8;
};

nalMatrix4x4 &stru_9771C0 = var<nalMatrix4x4>(0x009771C0);

#ifndef STANDALONE_SYSTEM
#error "Not defined macro STANDALONE_SYSTEM"
#endif

#if !STANDALONE_SYSTEM

tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString> *&nalSkeletonDirectory =
    var<tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString> *>(0x00977178);

tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString> *&nalAnimFileDirectory =
    var<tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString> *>(0x0097716C);

tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *&nalAnimDirectory =
    var<tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *>(0x00977170);

tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString> *&nalSceneAnimDirectory =
    var<tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString> *>(0x00977168);

static auto &dword_970D64 = var<void *>(0x00970D64);

char (&nalAnimPath)[255] = var<char[255]>(0x00976FC8);

char (&nalSkeletonPath)[255] = var<char[255]>(0x00976EC8);

static nalHeap &nalDefaultHeap = var<nalHeap>(0x00946A84);

static nalAnimCache &nalAnimationCache = var<nalAnimCache>(0x00977114);

static nalHeap *&nalAnimationHeap = var<nalHeap *>(0x00976EC0);

tlInstanceBank &nalTypeInstanceBank = var<tlInstanceBank>(0x009770E8);

tlInstanceBank &nalComponentInstanceBank = var<tlInstanceBank>(0x00977100);

LARGE_INTEGER &nalPlayerGetPoseTicks = var<LARGE_INTEGER>(0x009770D8);

#else

tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString> *&nalSkeletonDirectory = []() -> auto & {
    static tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString> *g_nalSkeletonDirectory{};
    return g_nalSkeletonDirectory;
}();

tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString> *&nalAnimFileDirectory = []() -> auto & {
    static tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString> *g_nalAnimFileDirectory{};
    return g_nalAnimFileDirectory;
}();

tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *&nalAnimDirectory = []() -> auto & {
    static tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *g_nalAnimDirectory{};
    return g_nalAnimDirectory;
}();

tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString> *&nalSceneAnimDirectory = []() -> auto & {
    static tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString> *g_nalSceneAnimDirectory{};
    return g_nalSceneAnimDirectory;
}();

static auto &dword_970D64 = []() -> auto & {
    static void *g_dword_970D64{};
    return g_dword_970D64;
}();

char (&nalAnimPath)[255] = []() -> auto & {
    static char g_nalAnimPath[255]{};
    return g_nalAnimPath;
}();

char (&nalSkeletonPath)[255] = []() -> auto & {
    static char g_nalSkeletonPath[255]{};
    return g_nalSkeletonPath;
}();

static nalHeap &nalDefaultHeap = []() -> auto & {
    static nalHeap g_nalDefaultHeap{};
    return g_nalDefaultHeap;
}();

static nalAnimCache &nalAnimationCache = []() -> auto & {
    static nalAnimCache g_nalAnimationCache{};
    return g_nalAnimationCache;
}();

static nalHeap *&nalAnimationHeap = []() -> auto & {
    static nalHeap *g_nalAnimationHeap{};
    return g_nalAnimationHeap;
}();

tlInstanceBank &nalTypeInstanceBank = []() -> auto & {
    static tlInstanceBank g_nalTypeInstanceBank{};
    return g_nalTypeInstanceBank;
}();

tlInstanceBank &nalComponentInstanceBank = []() -> auto & {
    static tlInstanceBank g_nalComponentInstanceBank{};
    return g_nalComponentInstanceBank;
}();

LARGE_INTEGER &nalPlayerGetPoseTicks = []() -> auto & {
    static LARGE_INTEGER g_nalPlayerGetPoseTicks{};
    return g_nalPlayerGetPoseTicks;
}();

#endif

int *nalComponentFloat1Base::_GetType()
{
    return &TypeID;
}

int *nalComponentFloat3Base::_GetType()
{
    return &TypeID;
}

int *nalComponentQuatBase::_GetType()
{
    return &TypeID;
}

int *nalComponentU8Base::_GetType()
{
    return &TypeID;
}

char *nalComponentStringBase::GetType()
{
    sp_log("%d", TypeID);

    return &TypeID;
}

nalBaseSkeleton *nalGetSkeleton(const tlFixedString &a1)
{
    nalBaseSkeleton *(__fastcall * Find)(void *, void *, const tlFixedString *) =
        CAST(Find, get_vfunc(nalSkeletonDirectory->m_vtbl, 0xC));

    return Find(nalSkeletonDirectory, nullptr, &a1);
}

void nalComponentBase::Process(const nalGeneric::nalComponentInfo *a1, void *&a2, void *&a3)
{
    void(__fastcall * func)(void *, void *edx, const nalGeneric::nalComponentInfo *, void **, void **) =
        CAST(func, get_vfunc(m_vtbl, 0x10));
    func(this, nullptr, a1, &a2, &a3);
}

void nalInit(nalHeap *a1)
{
    TRACE("nalInit");

    if constexpr (1) {
        tlStackRangeInit();
        if (tlScratchPadRefCount++ == 0) {
            dword_970D64 = tlMemAlloc(0x4000, 16, 0x2000000u);
        }

        nalAnimPath[0] = 0;
        nalSkeletonPath[0] = 0;

        nalAnimFileDirectory = new tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString>{};

        nalAnimDirectory = new tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString>{};

        nalSceneAnimDirectory = new tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString>{};

        nalSkeletonDirectory = new tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString>{};

        nalTypeInstanceBank.Init();
        nalComponentInstanceBank.Init();
        nalInitListInit();

        auto *v10 = a1;
        if (v10 == nullptr) {
            nalDefaultHeap.field_4 = 0x100000;
            nalDefaultHeap.field_8 = 0;
            v10 = &nalDefaultHeap;
        }

        nalAnimationCache.field_8 = 0;
        nalAnimationCache.field_4 = 0;
        nalAnimationHeap = v10;
        nalAnimationCache.field_0 = v10;

    } else {
        CDECL_CALL(0x00783CF0, a1);
    }
}

void nalExit()
{
    CDECL_CALL(0x00783C60);
}

void nalReleaseSceneAnimInternal(nalSceneAnim *a1)
{
    CDECL_CALL(0x0078D9B0, a1);
}

bool nalLoadSceneAnimInternal(nalSceneAnim *a1)
{
    return (bool)CDECL_CALL(0x0078D8D0, a1);
}

bool nalLoadAnimFileInternal(nalAnimFile *anim_file)
{
    TRACE("nalLoadAnimFileInternal", anim_file->field_10.to_string(), anim_file->field_48.to_string());

    if (anim_file->field_0 != 0x10101) {
        error("Unsupported anim file version %x, current version is %x.\n", anim_file->field_0, 0x10101);
    }

    if constexpr (1) {
        auto *v1 = &anim_file->field_48;
        auto **skeletons = static_cast<nalBaseSkeleton **>(tlMemAlloc(4 * anim_file->num_skeletons, 8, 0x2000000u));

        for (auto i = 0; i < anim_file->num_skeletons; ++i) {
            skeletons[i] = nalSkeletonDirectory->Find(v1[i]);
            if (skeletons[i] == nullptr) {
                auto v8 = anim_file->field_10.to_string();
                auto v3 = v1[i].to_string();
                error("The skeleton resource file %s was not found while loading animfile %s. "
                      "Perhaps something is wrong with the packer?\n",
                      v3,
                      v8);

                assert(0);
            }
        }

        nalAnimClass<nalAnyPose> *anim_class = nullptr;
        if (anim_file->field_34 != nullptr) {
            anim_file->field_34 =
                CAST(anim_file->field_34, bit_cast<char *>(anim_file->field_34) + (unsigned int)anim_file);
            anim_class = anim_file->field_34;
        }

        while (anim_class != nullptr) {
            if (anim_class->field_4 != nullptr) {
                anim_class->field_4 = CAST(anim_class->field_4, int(anim_class->field_4) + (unsigned int)anim_class);
            }

            auto *v7 = skeletons[anim_class->field_28];
            anim_class->Skeleton = v7;
            auto *instance = nalTypeInstanceBank.Search(v7->field_28);
            if (instance == nullptr) {
                assert(0 && "couldn't find animation type instance");
            }

            auto vtbl = static_cast<nalInitListAnimType *>(instance->field_20)->anim_vtbl_ptr;
            anim_class->m_vtbl = vtbl;

            if (!anim_class->CheckVersion()) {
                auto *v3 = &anim_class->field_8;
                auto *v9 = v3->to_string();
                auto v4 = anim_class->Version;
                error("Unsupported anim version %x (%s).\n", v4, v9);
            }

            anim_class->InstanceCount = 0;

            anim_class->Process();

            if (nalAnimDirectory->Add(anim_class)) {
                auto *v6 = anim_class->field_8.to_string();
                sp_log("Duplicate anim %s found.\n", v6);
            }

            anim_class = anim_class->field_4;
        }

        tlMemFree(skeletons);
        anim_file->field_4 |= 8u;
        return true;
    } else {
        bool (*func)(nalAnimFile *) = CAST(func, 0x0078D540);
        return func(anim_file);
    }
}

void nalSetSkeletonDirectory(tlResourceDirectory<nalBaseSkeleton, tlFixedString> *a1)
{
    nalSkeletonDirectory = CAST(nalSkeletonDirectory, a1);
}

tlInstanceBankResourceDirectory<nalBaseSkeleton, tlFixedString> *nalGetSkeletonDirectory()
{
    return nalSkeletonDirectory;
}

void nalSetAnimFileDirectory(tlResourceDirectory<nalAnimFile, tlFixedString> *a1)
{
    TRACE("nalSetAnimFileDirectory");

    nalAnimFileDirectory = CAST(nalAnimFileDirectory, a1);
}

tlInstanceBankResourceDirectory<nalAnimFile, tlFixedString> *nalGetAnimFileDirectory()
{
    return nalAnimFileDirectory;
}


void nalSetAnimDirectory(tlResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *a1)
{
    nalAnimDirectory = CAST(nalAnimDirectory, a1);
}

tlInstanceBankResourceDirectory<nalAnimClass<nalAnyPose>, tlFixedString> *nalGetAnimDirectory()
{
    return nalAnimDirectory;
}

void nalSetSceneAnimDirectory(tlResourceDirectory<nalSceneAnim, tlFixedString> *a1)
{
    nalSceneAnimDirectory = CAST(nalSceneAnimDirectory, a1);
}

tlInstanceBankResourceDirectory<nalSceneAnim, tlFixedString> *nalGetSceneAnimDirectory()
{
    return nalSceneAnimDirectory;
}

nalMatrix4x4::nalMatrix4x4(const nalPositionOrientation &a2)
{
    this->arr[0][3] = 0.0;
    this->arr[1][3] = 0.0;
    this->arr[2][3] = 0.0;
    this->arr[3][3] = 1.0;

    this->sub_5FC9C0(a2);

    this->arr[3][0] = a2.field_10[0];
    this->arr[3][1] = a2.field_10[1];
    this->arr[3][2] = a2.field_10[2];
}

nalMatrix4x4 sub_5FE000(const nalMatrix4x4 &arg4, const nalMatrix4x4 &arg8)
{
    nalMatrix4x4 result;

    if constexpr (0) {
        vector4d x_axis;
        x_axis[0] = arg8[0][0];
        x_axis[1] = arg8[0][1];
        x_axis[2] = arg8[0][2];
        x_axis[3] = arg8[0][3];

        vector4d y_axis;
        y_axis[0] = arg8[1][0];
        y_axis[1] = arg8[1][1];
        y_axis[2] = arg8[1][2];
        y_axis[3] = arg8[1][3];

        vector4d z_axis;
        z_axis[0] = arg8[2][0];
        z_axis[1] = arg8[2][1];
        z_axis[2] = arg8[2][2];
        z_axis[3] = arg8[2][3];

        vector4d w_axis;
        w_axis[0] = arg8[3][0];
        w_axis[1] = arg8[3][1];
        w_axis[2] = arg8[3][2];
        w_axis[3] = arg8[3][3];

        vector4d a3;
        a3[0] = arg4[0][0];
        a3[1] = arg4[0][1];
        a3[2] = arg4[0][2];
        a3[3] = arg4[0][3];

        vector4d a5;
        a5[0] = arg4[1][0];
        a5[1] = arg4[1][1];
        a5[2] = arg4[1][2];
        a5[3] = arg4[1][3];

        vector4d a7;
        a7[0] = arg4[2][0];
        a7[1] = arg4[2][1];
        a7[2] = arg4[2][2];
        a7[3] = arg4[2][3];

        vector4d arg8a;
        arg8a[0] = arg4[3][0];
        arg8a[1] = arg4[3][1];
        arg8a[2] = arg4[3][2];
        arg8a[3] = arg4[3][3];

        auto v16 = sub_4126E0(x_axis, a3, y_axis, a3, z_axis, a3);

        auto v21 = sub_4126E0(x_axis, a5, y_axis, a5, z_axis, a5);

        auto v25 = sub_4126E0(x_axis, a7, y_axis, a7, z_axis, a7);

        auto v29 = vector4d::sub_413E90(x_axis, arg8a, y_axis, arg8a, z_axis, arg8a, w_axis);

        nalMatrix4x4 result;
        result[0][0] = v16[0];
        result[0][1] = v16[1];
        result[0][2] = v16[2];
        result[0][3] = v16[3];
        result[1][0] = v21[0];
        result[1][1] = v21[1];
        result[1][2] = v21[2];
        result[1][3] = v21[3];
        result[2][0] = v25[0];
        result[2][1] = v25[1];
        result[2][2] = v25[2];
        result[2][3] = v25[3];
        result[3][0] = v29[0];
        result[3][1] = v29[1];
        result[3][2] = v29[2];
        result[3][3] = v29[3];
    } else {
        int(__cdecl * func)(nalMatrix4x4 *, const nalMatrix4x4 *, const nalMatrix4x4 *) = CAST(func, 0x005FE000);
        func(&result, &arg4, &arg8);
    }

    return result;
}

nalMatrix4x4 sub_5F2FD0(Float a2, const float *a3)
{
    if constexpr (0) {
    } else {
        void (*func)(nalMatrix4x4 *out, Float, const float *) = CAST(func, 0x005F2FD0);

        nalMatrix4x4 result{};
        func(&result, a2, a3);
        return result;
    }
}

float sub_5F4960(const nalMatrix4x4 &a2, bool a3)
{
    if constexpr (0) {
    } else {
        float (*func)(const nalMatrix4x4 *, bool) = CAST(func, 0x005F4960);
        return func(&a2, a3);
    }
}

vector4d sub_5FC4A0(const vector4d &a2, const float *a3, const vector4d &a4)
{
    vector4d result;
    result[0] = a2[0] * a3[2] + a4[0];
    result[1] = a2[1] * a3[2] + a4[1];
    result[2] = a2[2] * a3[2] + a4[2];
    result[3] = a2[3] * a3[2] + a4[3];
    return result;
}

vector4d sub_504170(const vector4d &a2, const float *a3, const vector4d &a4)
{
    vector4d result;
    result[0] = a2[0] * a3[1] + a4[0];
    result[1] = a2[1] * a3[1] + a4[1];
    result[2] = a2[2] * a3[1] + a4[2];
    result[3] = a2[3] * a3[1] + a4[3];
    return result;
}

vector4d sub_5E2F50(const vector4d &a2, const float *a3, const vector4d &a4)
{
    vector4d result;
    result[0] = a3[0] * a2[0] + a4[0];
    result[1] = a3[0] * a2[1] + a4[1];
    result[2] = a3[0] * a2[2] + a4[2];
    result[3] = a3[0] * a2[3] + a4[3];
    return result;
}

void sub_5FC820(const nalPositionOrientation &a1, vector4d &a2, vector4d &a3, vector4d &a4)
{
    vector4d v10{};
    v10[0] = a1.field_0[0] + a1.field_0[0];
    v10[1] = a1.field_0[1] + a1.field_0[1];
    v10[2] = a1.field_0[2] + a1.field_0[2];
    auto v5 = a1.field_0[3] + a1.field_0[3];
    v10[3] = v5;
    auto v11 = v5 * a1.field_0[0];
    auto v6 = v10[3] * a1.field_0[1];
    auto v12 = v10[3] * a1.field_0[2];
    auto v13 = v10[3] * a1.field_0[3];
    auto v16 = -v11;
    auto v17 = -v6;
    auto a1a = v13 - 1.0f;

    vector4d v15{};
    v15[0] = a1a;
    v15[1] = -v12;
    v15[2] = v6;
    v15[3] = 0.0f;
    a2 = sub_5E2F50(v10, a1.field_0, v15);

    v15[0] = v12;
    v15[1] = a1a;
    v15[2] = v16;
    v15[3] = 0.0f;
    a3 = sub_504170(v10, a1.field_0, v15);

    v15[0] = v17;
    v15[1] = v11;
    v15[2] = a1a;
    v15[3] = 0.0f;
    a4 = sub_5FC4A0(v10, a1.field_0, v15);
}

nalMatrix4x4::nalMatrix4x4(const nalMatrix4x4 &a2)
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *, void *edx, const nalMatrix4x4 *a2) = CAST(func, 0x005EBC90);
        func(this, nullptr, &a2);
    }
}


void nalMatrix4x4::sub_5FC9C0(const nalPositionOrientation &a2)
{
    if constexpr (0) {
        vector4d v10;
        vector4d v11;
        vector4d v12;
        sub_5FC820(a2, v10, v11, v12);

        this->arr[0][0] = v10[0];
        this->arr[0][1] = v10[1];
        this->arr[0][2] = v10[2];
        this->arr[0][3] = 0.0;

        this->arr[1][0] = v11[0];
        this->arr[1][1] = v11[1];
        this->arr[1][2] = v11[2];
        this->arr[1][3] = 0.0;

        this->arr[2][0] = v12[0];
        this->arr[2][1] = v12[1];
        this->arr[2][2] = v12[2];
        this->arr[2][3] = 0.0;
    } else {
        void(__fastcall * func)(void *self, void *edx, const float *a2) = CAST(func, 0x005FC9C0);
        func(this, nullptr, &a2.field_0[0]);
    }
}

nalMatrix4x4 nalMatrix4x4::sub_5EC0A0()
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(void *, void *edx, nalMatrix4x4 *out) = CAST(func, 0x005EC0A0);
        nalMatrix4x4 result{};
        func(this, nullptr, &result);
        return result;
    }
}

nalPositionOrientation::nalPositionOrientation(nalVector3 a2, const float *a3)
{
    this->field_0[0] = a3[0];
    this->field_0[1] = a3[1];
    this->field_0[2] = a3[2];
    this->field_0[3] = a3[3];
    this->field_10 = a2;
}

void DecomposeIKSpin(nalMatrix4x4 &a1, nalMatrix4x4 &a2, const nalMatrix4x4 &a3, const nalVector3 &a4,
                     const nalMatrix4x4 &a5, const IKSkelData &a6,
                     nalVector3 (*a7)(const nalMatrix4x4 &, const nalMatrix4x4 &, nalVector3), Float a8)
{
    TRACE("DecomposeIKSpin");

    if constexpr (0) {
    } else {
        void (*func)(nalMatrix4x4 *a1,
                     nalMatrix4x4 *a2,
                     const nalMatrix4x4 *a3,
                     const nalVector3 *a4,
                     const nalMatrix4x4 *a5,
                     const IKSkelData *a6,
                     nalVector3 (*a7)(const nalMatrix4x4 &, const nalMatrix4x4 &, nalVector3),
                     Float a8) = CAST(func, 0x005F16E0);
        func(&a1, &a2, &a3, &a4, &a5, &a6, a7, a8);
    }
}

nalVector3 LegHeuristic(const nalMatrix4x4 &, const nalMatrix4x4 &a3, nalVector3 a4)
{
    auto v4 = a3[1][0];
    auto v5 = a3[1][1];
    auto v6 = a3[1][2];

    nalVector3 result;
    result[0] = a4.field_0[1] * v6 - a4.field_0[2] * v5;
    result[1] = a4.field_0[2] * v4 - v6 * a4.field_0[0];
    result[2] = a4.field_0[0] * v5 - a4.field_0[1] * v4;
    return result;
}

nalMatrix4x4 sub_5F4170(Float a2, Float a3, const nalVector3 &a4)
{
    if constexpr (0) {
    } else {
        void (*func)(nalMatrix4x4 *a1, Float a2, Float a3, const nalVector3 *a4) = CAST(func, 0x005F4170);
        nalMatrix4x4 result{};
        func(&result, a2, a3, &a4);
        return result;
    }
}

void sub_5F42D0(nalMatrix4x4 &a1, nalMatrix4x4 &a2, Float a3, const nalVector3 &a4, const nalVector3 &a5, bool a6)
{
    if constexpr (0) {
    } else {
        void (*func)(nalMatrix4x4 *, nalMatrix4x4 *, Float, const nalVector3 *, const nalVector3 *, bool) =
            CAST(func, 0x005F42D0);
        func(&a1, &a2, a3, &a4, &a5, a6);
    }
}

nalVector3 LeftArmHeuristic(const nalMatrix4x4 &a2, const nalMatrix4x4 &a3, nalVector3 a4)
{
    if constexpr (0) {
    } else {
        nalVector3 result{};
        void (*func)(nalVector3 *out, const nalMatrix4x4 *a2, const nalMatrix4x4 *a3, nalVector3 a4) =
            CAST(func, 0x005EEEE0);
        func(&result, &a2, &a3, a4);
        return result;
    }
}

nalVector3 RightArmHeuristic(const nalMatrix4x4 &a2, const nalMatrix4x4 &a3, nalVector3 a4)
{
    if constexpr (0) {
    } else {
        nalVector3 result{};
        void (*func)(nalVector3 *out, const nalMatrix4x4 *a2, const nalMatrix4x4 *a3, nalVector3 a4) =
            CAST(func, 0x005EF100);
        func(&result, &a2, &a3, a4);
        return result;
    }
}

void nalStreamInstance_patch()
{
    REDIRECT(0x005AD21F, nalInit);

    REDIRECT(0x0055F8F4, nalConstructSkeleton);
    return;


    {
        FUNC_ADDRESS(address, &nalGeneric::nalGenericSkeleton::Process);
        set_vfunc(0x008BD3D8, address);
    }

    {
        FUNC_ADDRESS(address, &nalComponentU8Base::_GetType);
        SET_JUMP(0x004AE4C0, address);
    }

    {
        FUNC_ADDRESS(address, &nalComponentStringBase::GetType);
        SET_JUMP(0x004AE4D0, address);
    }

    {
        FUNC_ADDRESS(address, &nalComponentInitList::Register);
        //set_vfunc(0x00880958, address);
    }

    {
        FUNC_ADDRESS(address, &nalComp::nalCompSkeleton::UnMash);
        set_vfunc(0x00891FC8, address);
        set_vfunc(0x008AA300, address);
    }

#if 0
    {
        FUNC_ADDRESS(address, &nalStreamInstance::IsReady);
        set_vfunc(0x00880A8C, address);
    }

    //nalStreamInstance::Advance
    {
        REDIRECT(0x004985FB, nflReadFileAsync);

        REDIRECT(0x00498622, nflGetRequestInfo);

        REDIRECT(0x0049862B, nflGetRequestState);

        REDIRECT(0x004986FF, tlMemAlloc);
    }

    {
        FUNC_ADDRESS(address, &nalStreamInstance::Advance);
        //set_vfunc(0x00880A90, address);
    }

    {
        FUNC_ADDRESS(address, &nalStreamInstance::AdvanceStream);
        REDIRECT(0x00498943, address);
    }
#endif
}
