#pragma once

#include "hashstring.h"
#include "nal_skeleton.h"
#include "variable.h"

#include <float.hpp>

#include <cstdint>

struct nalComponentBase;
struct nalMatrix4x4;

namespace nalGeneric {

struct nalGenericSkeleton;
struct nalGenericPose;
struct nalGenericInstance;


struct nalGenericAnim {
    std::intptr_t m_vtbl;
    int field_4[9];

    tlHashString field_28;
    unsigned int field_2C;
    nalGenericSkeleton *field_30;
    int field_34;
    int field_38;
    int field_3C;
    int field_40;
    int field_44;
    int field_48;
    int field_4C;
    int field_50;
    int field_54;
    int field_58;
    void *field_5C;
    uint32_t *field_60;
    int field_64;
    int field_68;
    void **field_6C;
    int field_70;
    int field_74;
    void **field_78;
    int field_7C;
    char field_80;

    struct vtbl {};

    static int &vtbl_ptr;

    void Process();
    void Release();
    bool CheckVersion() const;
    nalGenericInstance *CreateInstance(nalGenericSkeleton *skeleton);
};

struct nalComponentInfo {
    tlHashString field_0;
    char field_4[0x1C];
    nalComponentBase *field_20;
    int field_24;
    int field_28;
    int field_2C;
};

template <uint32_t I>
struct MorphSliderPoseTemplate {
    int field_0{0xFF};
    int field_4;
    int field_8;
    int field_C;
};

template <typename T>
struct nalGenericComponentHandle {
    nalGenericSkeleton *Skeleton{nullptr};
    int field_4;
    int field_8;
    int field_C;
};

template <typename T>
struct nalGenericConstComponentHandle {
    nalGenericSkeleton *Skeleton{nullptr};
    struct {
        char field_0[0x2C];
        T *field_2C;
    } *field_4;
    int field_8;
    int field_C;
};

struct nalGenericPose {
    nalGenericSkeleton *field_0;
    int field_4;
    bool field_8;

    nalGenericPose();

    //0x00794110
    nalGenericPose(const nalGenericSkeleton *a3);

    //0x007941F0
    nalGenericPose(const nalGeneric::nalGenericPose &a3, bool a4);
    ~nalGenericPose();
    nalGenericPose &operator=(const nalGenericPose &source);
    void ConstructEmptyData();

    auto *GetSkeleton() const
    {
        return this->field_0;
    }

    template <typename T>
    T *operator[](nalGeneric::nalGenericComponentHandle<T> &handle)
    {
        assert(handle.Skeleton != nullptr && "attempting to de-reference an invalid handle");

        assert(handle.Skeleton == GetSkeleton() && "handle and pose skeletons don't match");
        auto *info = bit_cast<const nalComponentInfo *>(handle.field_4);
        return bit_cast<T *>(this->field_4 + info->field_2C + sizeof(T) * handle.field_8);
    }

    template <typename T>
    T operator[](nalGenericConstComponentHandle<T> &handle)
    {
        assert(handle.Skeleton != nullptr && "attempting to de-reference an invalid handle");

        auto *skeleton = this->GetSkeleton();
        assert(handle.Skeleton == skeleton && "handle and pose skeletons don't match");

        if (handle.field_C) {
            return (*skeleton)[handle];
        }

        return *bit_cast<T *>(this->field_4 + bit_cast<const nalComponentInfo *>(handle.field_4)->field_2C
                             + sizeof(T) * handle.field_8);
    }

    static int &PoseSP;

    static int &PoseStack;
};

struct nalGenericInstance {
    std::intptr_t m_vtbl;
    float field_4;
    float field_8;
    nalGenericSkeleton *field_C;
    nalGenericAnim *field_10;
    union {
        nalGenericPose field_14;
    };
    float field_20;
    int field_24;
    struct OffsetMap *field_28;

    nalGenericInstance(nalGenericAnim *anim, nalGenericSkeleton *skeleton);
    ~nalGenericInstance();
    void Finalize(bool release);
    void GetPose(Float time, Float previous, nalGenericPose &out, const nalGenericPose &reference);
    void GetFrame(int frame, nalGenericPose &out, const nalGenericPose &reference);
    void CacheBlock(int block);
};

void Blend(nalGenericPose *out, float weight, const nalGenericPose *a, const nalGenericPose *b);

struct nalGenericSkeleton : nalBaseSkeleton {
    int field_5C;
    int field_60;
    int field_64;
    int field_68;
    int field_6C;
    int field_70;
    int field_74;
    int field_78;
    int field_7C;
    int field_80;
    int field_84;
    int field_88;
    nalComponentInfo *field_8C;
    int field_90;
    int field_94;
    int field_98;
    int field_9C;
    int field_A0;
    int field_A4;
    nalComponentInfo *field_A8;
    int field_AC;
    int field_B0;
    int field_B4;
    int field_B8;
    int field_BC;
    int field_C0;
    int field_C4;
    int field_C8;
    nalGeneric::nalGenericPose field_CC;
    int field_D8;
    int field_DC;
    int field_E0;

    template <typename T>
    void GetComponentHandle(nalGenericComponentHandle<T> &a2, tlFixedString &a3, tlFixedString &a4);

    template <typename T>
    void GetComponentHandle(nalGenericConstComponentHandle<T> &a2, tlFixedString &a3, tlFixedString &a4) const;

    template <typename T>
    void GetComponentHandle(nalGenericConstComponentHandle<T> &a2, uint32_t a3, tlFixedString &a4) const;

    nalGenericSkeleton();

    //0x00794CF0
    nalMatrix4x4 *GetBoneMatrices(const nalGenericPose *a2, nalMatrix4x4 *a3) const;
    void GetTrajectoryData(const nalGenericPose *pose, nalPositionOrientation *out) const;
    nalGenericPose *CreatePose() const;
    void DestroyPose(nalGenericPose *pose) const;
    nalGenericPose *GetDefaultPose();
    void CopyPose(nalGenericPose &out, const nalGenericPose &source) const;
    void BlendPose(nalGenericPose &out, Float weight, const nalGenericPose &a, const nalGenericPose &b) const;
    void GetPoseFromBoneMatrices(nalGenericPose &out, const nalMatrix4x4 *source,
                                 nalMatrix4x4 *scratch, const nalGenericPose &reference) const;
    int GetBoneCount() const { return field_60; }
    void Finalize(bool release);

    //virtual
    //0x00793610
    void _Process();

    void _Release();

    bool _CheckVersion() const
    {
        return this->Version == 0x10200;
    }

    template <typename T>
    T operator[](nalGenericConstComponentHandle<T> &handle)
    {
        assert(handle.Skeleton != nullptr && "attempting to de-reference an invalid handle");

        assert(handle.Skeleton == this && "handle and pose skeletons don't match");

        auto *v4 = bit_cast<char *>(bit_cast<const nalComponentInfo *>(handle.field_4)->field_2C
                                    + sizeof(T) * handle.field_8);
        if (handle.field_C) {
            return *bit_cast<T *>(&v4[this->field_B4]);
        } else {
            return *bit_cast<T *>(&v4[this->field_98]);
        }
    }

    static int &vtbl_ptr;
};

}  // namespace nalGeneric

extern void nalGeneric_patch();
