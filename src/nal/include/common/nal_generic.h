#pragma once

#include "hashstring.h"
#include "nal_skeleton.h"
#include "variable.h"

#include <float.hpp>

#include <cstdint>

struct nalMatrix4x4;

namespace nalGeneric {

struct nalGenericSkeleton;
struct nalGenericPose;

struct nalGenericInstance {

    //0x007946A0
    void GetPose(
        Float a2,
        Float a3,
        nalGeneric::nalGenericPose &a4,
        const nalGeneric::nalGenericPose &a5);
};

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

    struct vtbl {};

    static int & vtbl_ptr;
};

struct nalComponentInfo {
    tlHashString field_0;
    char field_4[0x1C];
    struct {
        std::intptr_t m_vtbl;
    } * field_20;
    int field_24;
    int field_28;
    int field_2C;
};

template<uint32_t I>
struct MorphSliderPoseTemplate {
    int field_0 {0xFF};
    int field_4;
    int field_8;
    int field_C;
};

template<typename T>
struct nalGenericComponentHandle {
    nalGeneric::nalGenericSkeleton *Skeleton {nullptr};
    int field_4;
    int field_8;
    int field_C;

};

template<typename T>
struct nalGenericConstComponentHandle {
    nalGeneric::nalGenericSkeleton *Skeleton {nullptr};
    struct {
        char field_0[0x2C];
        T *field_2C;
    } * field_4;
    int field_8;
    int field_C;
};


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
    int field_CC;
    int field_D0;
    int field_D4;
    int field_D8;
    int field_DC;
    int field_E0;

    template<typename T>
    void GetComponentHandle(
        nalGenericComponentHandle<T> &a2,
        tlFixedString &a3,
        tlFixedString &a4);

    template<typename T>
    void GetComponentHandle(
        nalGenericConstComponentHandle<T> &a2,
        tlFixedString &a3,
        tlFixedString &a4) const;

    template<typename T>
    void GetComponentHandle(
        nalGenericConstComponentHandle<T> &a2,
        uint32_t a3,
        tlFixedString &a4) const;

    struct vtbl {
        void *field_0;
        void *field_4;
        void (nalGenericSkeleton::*Process)();
        void (nalGenericSkeleton::*Release)();
        bool (nalGenericSkeleton::*CheckVersion)();
    };

    nalGenericSkeleton();

    //0x00794CF0
    nalMatrix4x4 * GetBoneMatrices(
        const nalGenericPose *a2,
        nalMatrix4x4 *a3) const;

    //0x00793610
    void Process();

    void Release();

    bool CheckVersion() {
        return this->Version == 0x10200;
    }

    template<typename T>
    T operator[](nalGeneric::nalGenericConstComponentHandle<T> &handle)
    {
        static T g_invalidObject {};

        assert(handle.Skeleton != nullptr && "attempting to de-reference an invalid handle");

        if ( handle.Skeleton == nullptr ) {
            return g_invalidObject;
        }

        assert(handle.Skeleton == this && "handle and pose skeletons don't match");

        auto *v4 = bit_cast<char *>(handle.field_4->field_2C + handle.field_8);
        if (handle.field_C) {
            return *bit_cast<T *>(&v4[this->field_B4]);
        } else {
            return *bit_cast<T *>(&v4[this->field_98]);
        }
    }

    static int & vtbl_ptr;
};

struct nalGenericPose {
    nalGenericSkeleton *field_0;
    int field_4;
    bool field_8;

    //0x00794110
    nalGenericPose(const nalGenericSkeleton *a3);

    //0x007941F0
    nalGenericPose(
        const nalGeneric::nalGenericPose &a3,
        bool a4);

    auto * GetSkeleton() const {
        return this->field_0;
    }

    template<typename T>
    T * operator[](nalGeneric::nalGenericComponentHandle<T> &handle)
    {
        static T g_invalidObject {};

        assert(handle.Skeleton != nullptr && "attempting to de-reference an invalid handle");

        if ( handle->Skeleton == nullptr ) {
            return &g_invalidObject;
        }

        assert(handle.Skeleton == GetSkeleton() && "handle and pose skeletons don't match");

        return bit_cast<T *>(handle->field_4->field_2C + 12 * handle->field_8 + this->field_4);
    }

    template<typename T>
    T operator[](nalGeneric::nalGenericConstComponentHandle<T> &handle)
    {
        static T g_invalidObject {};

        assert(handle.Skeleton != nullptr && "attempting to de-reference an invalid handle");

        if ( handle.Skeleton == nullptr ) {
            return g_invalidObject;
        }

        auto *skeleton = this->GetSkeleton();
        assert(handle.Skeleton == skeleton && "handle and pose skeletons don't match");

        if (handle.field_C) {
            return (*skeleton)[handle];
        }

        return *bit_cast<T *>(handle.field_4->field_2C + 12 * handle.field_8 + this->field_4);
    }

    static int &PoseSP;

    static int &PoseStack;
};

} // nalGeneric
