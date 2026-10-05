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

#include <functional>
#include <cassert>
#include <cmath>
#include "matrix4x4.h"
#include "vector3d.h"

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

namespace {
struct CacheEntry {
    void **owner;
    CacheEntry *previous;
    CacheEntry *next;
    int size;
};

nalHeap *__fastcall HeapFinalize(nalHeap *heap, void *, bool release)
{
    if (release)
        ::operator delete(heap);
    return heap;
}

void *__fastcall HeapAllocate(nalHeap *heap, void *, int size, int alignment)
{
    if (static_cast<uint32_t>(heap->field_8 + size) > heap->field_4)
        return nullptr;
    heap->field_8 += size;
    return tlMemAlloc(size, alignment, 0x2000000u);
}

void __fastcall NativeHeapFree(nalHeap *heap, void *, void *data, int size)
{
    heap->field_8 -= size;
    tlMemFree(data);
}
}

void nalCacheFree(void *data)
{
    if (!data)
        return;
    auto *entry = static_cast<CacheEntry *>(data) - 1;
    if (entry->owner)
        *entry->owner = nullptr;
    if (entry->previous)
        entry->previous->next = entry->next;
    else
        nalAnimationCache.field_8 = reinterpret_cast<int>(entry->next);
    if (entry->next)
        entry->next->previous = entry->previous;
    else
        nalAnimationCache.field_4 = reinterpret_cast<int>(entry->previous);
    auto *heap = nalAnimationCache.field_0;
    auto free = reinterpret_cast<void(__fastcall *)(nalHeap *, void *, void *, int)>(
        get_vfunc(heap->m_vtbl, 8));
    free(heap, nullptr, entry, entry->size);
}

void nalCacheTouch(void *data)
{
    if (!data)
        return;
    auto *entry = static_cast<CacheEntry *>(data) - 1;
    if (reinterpret_cast<int>(entry) == nalAnimationCache.field_8)
        return;
    if (entry->previous)
        entry->previous->next = entry->next;
    if (entry->next)
        entry->next->previous = entry->previous;
    else
        nalAnimationCache.field_4 = reinterpret_cast<int>(entry->previous);
    entry->previous = nullptr;
    entry->next = reinterpret_cast<CacheEntry *>(nalAnimationCache.field_8);
    if (entry->next)
        entry->next->previous = entry;
    else
        nalAnimationCache.field_4 = reinterpret_cast<int>(entry);
    nalAnimationCache.field_8 = reinterpret_cast<int>(entry);
}

void *nalCacheAllocate(int size, int alignment, void **owner)
{
    auto *heap = nalAnimationCache.field_0;
    auto allocate = reinterpret_cast<void *(__fastcall *)(nalHeap *, void *, int, int)>(
        get_vfunc(heap->m_vtbl, 4));
    CacheEntry *entry;
    while (!(entry = static_cast<CacheEntry *>(allocate(
                 heap, nullptr, size + sizeof(CacheEntry), alignment > 4 ? alignment : 4)))) {
        nalCacheFree(reinterpret_cast<CacheEntry *>(nalAnimationCache.field_4) + 1);
    }
    entry->size = size + sizeof(CacheEntry);
    entry->owner = owner;
    if (owner)
        *owner = entry + 1;
    entry->previous = nullptr;
    entry->next = reinterpret_cast<CacheEntry *>(nalAnimationCache.field_8);
    if (entry->next)
        entry->next->previous = entry;
    else
        nalAnimationCache.field_4 = reinterpret_cast<int>(entry);
    nalAnimationCache.field_8 = reinterpret_cast<int>(entry);
    return entry + 1;
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
            static void *heap_table[]{reinterpret_cast<void *>(&HeapFinalize),
                                      reinterpret_cast<void *>(&HeapAllocate),
                                      reinterpret_cast<void *>(&NativeHeapFree)};
            nalDefaultHeap.m_vtbl = reinterpret_cast<std::intptr_t>(heap_table);
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
    nalSceneAnimDirectory->ReleaseAll(false, false, 1);
    nalAnimFileDirectory->ReleaseAll(false, false, 1);
    nalSkeletonDirectory->ReleaseAll(false, false, 1);



    while (nalAnimationCache.field_8 != 0) {
        auto *entry = reinterpret_cast<CacheEntry *>(nalAnimationCache.field_8);
        nalAnimationCache.field_8 = reinterpret_cast<int>(entry->next);
        auto *heap = nalAnimationCache.field_0;
        auto free = reinterpret_cast<void(__fastcall *)(nalHeap *, void *, void *, int)>(
            get_vfunc(heap->m_vtbl, 8));
        free(heap, nullptr, entry, entry->size);
    }



    nalTypeInstanceBank.Release();
    nalComponentInstanceBank.Release();

    auto destroy_directory = [](auto *directory) {
        if (directory != nullptr) {
            auto finalize = reinterpret_cast<void(__fastcall *)(void *, void *, bool)>(
                get_vfunc(directory->m_vtbl, 0));
            finalize(directory, nullptr, true);
        }
    };
    destroy_directory(nalSkeletonDirectory);
    destroy_directory(nalSceneAnimDirectory);
    destroy_directory(nalAnimDirectory);
    destroy_directory(nalAnimFileDirectory);

    if (--tlScratchPadRefCount == 0) {
        tlMemFree(dword_970D64);
        dword_970D64 = nullptr;
    }
}

void nalReleaseSceneAnimInternal(nalSceneAnim *scene_anim)
{
    auto *node = *bit_cast<std::intptr_t **>(bit_cast<char *>(scene_anim) + 0x34);
    while (node != nullptr) {
        auto *anim = bit_cast<nalAnimClass<nalAnyPose> *>(node[2]);
        while (anim != nullptr) {
            auto *next = anim->field_4;
            void(__fastcall *release)(void *) = CAST(release, get_vfunc(anim->m_vtbl, 0x8));
            release(anim);
            anim = next;
        }
        node = bit_cast<std::intptr_t *>(node[0]);
    }
}

bool nalLoadSceneAnimInternal(nalSceneAnim *scene_anim)
{
    const auto skeleton_count = scene_anim->field_C;
    auto **skeletons =
        static_cast<nalBaseSkeleton **>(tlMemAlloc(4 * skeleton_count, 8, 0x2000000u));
    auto *skeleton_names = &scene_anim->field_50;
    for (int i = 0; i < skeleton_count; ++i) {
        skeletons[i] = nalSkeletonDirectory->Find(skeleton_names[i]);
    }

    auto *base = bit_cast<char *>(scene_anim);
    auto &head = *bit_cast<std::intptr_t **>(base + 0x34);
    if (head != nullptr) {
        head = bit_cast<std::intptr_t *>(base + bit_cast<std::intptr_t>(head));
        for (auto *node = head; node != nullptr; node = bit_cast<std::intptr_t *>(node[0])) {
            if (node[0] != 0) {
                node[0] += bit_cast<std::intptr_t>(node);
            }
            if (node[2] != 0) {
                node[2] += bit_cast<std::intptr_t>(node);
            }

            auto *anim = bit_cast<nalAnimClass<nalAnyPose> *>(node[2]);
            while (anim != nullptr) {
                if (anim->field_4 != nullptr) {
                    anim->field_4 =
                        bit_cast<nalAnimClass<nalAnyPose> *>(bit_cast<char *>(anim)
                                                            + bit_cast<std::intptr_t>(anim->field_4));
                }

                auto *skeleton = skeletons[anim->field_28];
                anim->Skeleton = skeleton;
                auto *instance = nalTypeInstanceBank.Search(skeleton->GetAnimTypeName());
                assert(instance != nullptr && "couldn't find scene animation type instance");
                anim->m_vtbl =
                    static_cast<nalInitListAnimType *>(instance->field_20)->anim_vtbl_ptr;
                assert(anim->CheckVersion() && "unsupported scene animation version");
                anim->Process();
                anim = anim->field_4;
            }
        }
    }

    tlMemFree(skeletons);
    return true;
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
            auto *instance = nalTypeInstanceBank.Search(v7->GetAnimTypeName());
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

nalMatrix4x4 sub_5FE000(const nalMatrix4x4 &local, const nalMatrix4x4 &parent)
{
    nalMatrix4x4 result;
    nalComposeMatrices(result, local, parent);
    return result;
}


float nalPoseCos(float angle)
{
    const float phase = -std::fabs(angle) * 0.15915493667125702f;
    const float t = std::fabs(std::ceil(phase) - phase - 0.5f) - 0.25f;
    const float t2 = t * t;
    const float t3 = t2 * t;
    const float t4 = t2 * t2;
    const float t5 = t4 * t;
    const float t7 = t3 * t4;
    const float t9 = t5 * t4;
    float result = t9 * 39.71065902709961f;
    result += t7 * -76.57495880126953f;
    result += t5 * 81.60222625732422f;
    result += t3 * -41.3416748046875f;
    return result + t * 6.283185005187988f;
}

float nalPoseSin(float angle)
{
    return nalPoseCos(angle + 4.71238899230957f);
}

namespace {
float poseAsin(float t)
{
    if (t < 0.5f) {
        const double t2 = double(t) * t;
        const double t3 = t2 * t;
        const double t5 = t3 * t2;
        const double t7 = t5 * t2;
        return static_cast<float>(t7 * 0.0539812408387661f + t5 * 0.07500000298023224f +
                                  t3 * 0.16666670143604279f + t);
    }
    const double u = std::sqrt(std::fabs((1.0 - t) * 0.5));
    const double u2 = u * u;
    const double u3 = u2 * u;
    const double u5 = u3 * u2;
    const double u7 = u5 * u2;
    return static_cast<float>(u7 * -0.10796249657869339f - u5 * 0.15000000596046448f -
                              u3 * 0.3333333134651184f - 2.0 * u + 1.570796012878418f);
}

nalMatrix4x4 poseIdentity()
{
    nalMatrix4x4 result{};
    for (int i = 0; i != 4; ++i)
        result[i][i] = 1.0f;
    return result;
}
}

nalMatrix4x4 sub_5F2FD0(Float angle, const float *position)
{
    const float c = nalPoseCos(angle);
    const float s = nalPoseSin(angle);
    nalMatrix4x4 result = poseIdentity();
    result[2][1] = -s;
    result[1][1] = c;
    result[2][2] = c;
    result[1][2] = s;
    for (int i = 0; i != 3; ++i)
        result[3][i] = position[i];
    return result;
}

float sub_5F4960(const nalMatrix4x4 &source, bool left)
{
    nalMatrix4x4 aligned = source;
    const float x = source[0][0];
    if (-0.9900000095367432f < x && x < 0.9900000095367432f) {
        const float invLength = 1.0f / std::sqrt(source[0][2] * source[0][2] +
                                                source[0][1] * source[0][1]);
        float arc = poseAsin(std::fabs(x));
        if (x < 0.0f)
            arc = -arc;
        const float half = (1.5707963705062866f - arc) * 0.5f;
        const float s = nalPoseSin(half);
        const float q[4] = {0.0f, -source[0][2] * invLength * s,
                           source[0][1] * invLength * s, nalPoseCos(half)};
        const nalMatrix4x4 rotation(nalPositionOrientation(nalVector3{}, q));
        aligned = sub_5FE000(aligned, rotation);
        for (int i = 0; i != 4; ++i)
            aligned[3][i] = source[3][i];
    }
    const float q[4] = {left ? -0.70710677f : 0.70710677f, 0.0f, 0.0f, 0.70710677f};
    const nalMatrix4x4 rotation(nalPositionOrientation(nalVector3{}, q));
    aligned = sub_5FE000(aligned, rotation);
    const float y = std::asin(aligned[0][2]);
    const float c = nalPoseCos(y);
    if (std::fabs(c) <= 0.004999999888241291f)
        return -0.0f;
    return -std::atan2(-aligned[1][2] / c, aligned[2][2] / c);
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

nalMatrix4x4::nalMatrix4x4(const nalMatrix4x4 &a2) = default;

void nalMatrix4x4::sub_5FC9C0(const nalPositionOrientation &a2)
{
    vector4d rows[3];
    sub_5FC820(a2, rows[0], rows[1], rows[2]);
    for (int i = 0; i != 3; ++i) {
        for (int j = 0; j != 3; ++j)
            arr[i][j] = rows[i][j];
        arr[i][3] = 0.0f;
    }
}

nalMatrix4x4 nalMatrix4x4::sub_5EC0A0()
{
    const auto cofactor = [this](int row, int col) {
        float m[9];
        int n = 0;
        for (int i = 0; i != 4; ++i)
            if (i != row)
                for (int j = 0; j != 4; ++j)
                    if (j != col)
                        m[n++] = arr[i][j];
        const double value = double(m[7]) * m[3] * m[2] + double(m[6]) * m[5] * m[1] +
                             double(m[8]) * m[4] * m[0] - double(m[2]) * m[6] * m[4] -
                             double(m[7]) * m[5] * m[0] - double(m[3]) * m[1] * m[8];
        return ((row ^ col) & 1) ? -value : value;
    };
    float determinant = 0.0f;
    for (int i = 0; i != 4; ++i)
        determinant = static_cast<float>(determinant + cofactor(0, i) * arr[0][i]);

    if (std::equal_to<float>{}(determinant, 0.0f))
        return poseIdentity();
    const float inverseDeterminant = 1.0f / determinant;
    nalMatrix4x4 result;
    for (int i = 0; i != 4; ++i)
        for (int j = 0; j != 4; ++j)
            result[j][i] = static_cast<float>(cofactor(i, j)) * inverseDeterminant;
    return result;
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
    vector3d target{a5[3][0], a5[3][1], a5[3][2]};
    vector3d origin;
    vector3d axis;
    float sin0, cos0, sin1, cos1;
    inverse_kinematics::nalIKSolve2D(
        reinterpret_cast<matrix4x4 *>(const_cast<nalMatrix4x4 *>(&a3)),
        reinterpret_cast<vector3d *>(const_cast<nalVector3 *>(&a4)), &target,
        a6.field_0, a6.field_8, a6.field_4, a6.field_C, &origin, &axis, &sin0, &cos0, &sin1, &cos1);
    nalVector3 direction;
    direction[0] = axis.x;
    direction[1] = axis.y;
    direction[2] = axis.z;
    const nalVector3 bend = a7(a3, a5, direction);
    vector4d bendDirection{};
    for (int i = 0; i != 3; ++i)
        bendDirection[i] = bend[i];
    inverse_kinematics::nalIKMap2DTo3D(
        a6.field_10, sin0, cos0, sin1, cos1, &origin, &axis, &bendDirection,
        std::sin(a8.value), std::cos(a8.value), reinterpret_cast<matrix4x4 *>(&a1),
        reinterpret_cast<matrix4x4 *>(&a2));
    for (nalMatrix4x4 *joint : {&a1, &a2}) {
        for (int i = 0; i != 4; ++i) {
            (*joint)[0][i] = -(*joint)[0][i];
            const float oldY = (*joint)[1][i];
            (*joint)[1][i] = -(*joint)[2][i];
            (*joint)[2][i] = -oldY;
        }
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

void ReconstituteBaseKnuckle(nalMatrix4x4 &a1, Float a2, Float a3, const nalVector3 &a4)
{
    const float sy = static_cast<float>(std::sin(double(a2.value) * 0.5));
    const float cy = static_cast<float>(std::cos(double(a2.value) * 0.5));
    const float sz = static_cast<float>(std::sin(double(a3.value) * 0.5));
    const float cz = static_cast<float>(std::cos(double(a3.value) * 0.5));
    const float q[4] = {sy * sz, sy * cz, cy * sz, cy * cz};
    a1 = nalMatrix4x4(nalPositionOrientation(a4, q));
}

void Unconvert2Knuckle(nalMatrix4x4 &a1, nalMatrix4x4 &a2, Float a3, const nalVector3 &a4, const nalVector3 &a5,
                       bool a6)
{
    sub_5F3080(a1, a3, a4);
    if (a6) {
        a3 = a3.value * 2.0f;
        if (a3 > 1.570796012878418f)
            a3 = 1.570796012878418f;
        else if (a3 < -1.570796012878418f)
            a3 = -1.570796012878418f;
    }
    sub_5F3080(a2, a3, a5);
}

void ReconstituteFingerCurl(nalMatrix4x4 &a1, nalMatrix4x4 &a2, nalMatrix4x4 &a3, const nalVector3 &a4,
                            const nalVector3 &a5, const nalVector3 &a6, Float a7, Float a8)
{
    ReconstituteBaseKnuckle(a1, a7, a8, a4);
    sub_5F3080(a2, a7, a5);
    sub_5F3080(a3, a7, a6);
}

namespace {
nalVector3 armHeuristic(const nalMatrix4x4 &matrix, nalVector3 direction, bool mirrored)
{
    nalVector3 result = LegHeuristic(matrix, matrix, direction);
    float dot = direction[0] * matrix[1][0] + direction[1] * matrix[1][1] +
                direction[2] * matrix[1][2];
    if (mirrored)
        dot = -dot;
    for (int i = 0; i != 3; ++i) {
        if (dot < 0.0f)
            result[i] = result[i] * (dot + 1.0f) + (-matrix[0][i] - matrix[2][i]) * -dot;
        else
            result[i] = result[i] * (1.0f - dot) + (matrix[2][i] - matrix[0][i]) * dot;
    }
    return result;
}
}

nalVector3 LeftArmHeuristic(const nalMatrix4x4 &a2, const nalMatrix4x4 &, nalVector3 a4)
{
    return armHeuristic(a2, a4, false);
}

nalVector3 RightArmHeuristic(const nalMatrix4x4 &a2, const nalMatrix4x4 &, nalVector3 a4)
{
    return armHeuristic(a2, a4, true);
}

void sub_5F3080(nalMatrix4x4 &a1, Float a2, const nalVector3 &a3)
{
    const float c = nalPoseCos(a2);
    const float s = nalPoseSin(a2);
    a1 = poseIdentity();
    a1[2][0] = -s;
    a1[0][0] = c;
    a1[2][2] = c;
    a1[0][2] = s;
    for (int i = 0; i != 3; ++i)
        a1[3][i] = a3[i];
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
