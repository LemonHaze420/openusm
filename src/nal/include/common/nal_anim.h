#pragma once

#include "fixedstring.h"

#include <float.hpp>
#include <vtbl.h>

#include <cstdint>

struct nalBaseSkeleton;
struct nalBasePose;

struct nalAnyPose {
    nalBasePose *field_0;

    nalAnyPose() = default;

    nalAnyPose(nalBaseSkeleton *a2);

    nalAnyPose(const nalBasePose &a2, bool a3);

    nalAnyPose(const nalAnyPose &a2, bool a3);

    const nalBaseSkeleton *GetSkeleton() const;

    void operator=(const nalAnyPose &a2);
};

template <typename T>
struct nalAnimClass {
    struct nalInstanceClass {
        std::intptr_t m_vtbl;
        float field_4;
        float field_8;
        nalBaseSkeleton *field_C;
        nalAnimClass<T> *field_10{nullptr};

        using nalPose = T;

        nalInstanceClass(nalAnimClass<T> *a2, nalBaseSkeleton *a3);

        void *operator new(size_t size);

        void operator delete(void *ptr);

        const nalBaseSkeleton *GetSkeleton() const
        {
            return this->field_C;
        }

        void GetPose(Float a2, Float a3, nalAnyPose &pose, const nalAnyPose &defaultPose);

        //virtual
        ~nalInstanceClass();

        //0x00796D50
        void finalize(bool a2);

        //virtual
        void VirtualGetPose(Float t, Float t_prev, nalBasePose &pose, const nalBasePose &defaultPose);  // = 0;
    };

    std::intptr_t m_vtbl;
    nalAnimClass<T> *field_4;
    tlFixedString field_8;
    int field_28;
    int Version;
    nalBaseSkeleton *Skeleton;
    int field_34;
    float field_38;
    int InstanceCount;

    auto *GetSkeleton()
    {
        return this->Skeleton;
    }

    nalInstanceClass *CreateInstance(nalBaseSkeleton *skeleton);

    void _Process() {}

    //virtual
    void Process()
    {
        void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
        func(this);
    }

    void _Release() {}

    //virtual
    void Release();

    //virtual
    bool _CheckVersion() const
    {
        return false;
    }

    bool CheckVersion() const
    {
        bool(__fastcall * func)(const void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
        return func(this);
    }

    //virtual
    nalInstanceClass *VirtualCreateInstance(nalBaseSkeleton *Skel);

    static tlFixedString *get_string(nalAnimClass<T> *a1)
    {
        return &a1->field_8;
    }
};

struct nalBaseInstance : nalAnimClass<nalAnyPose>::nalInstanceClass {
    nalBaseInstance(nalAnimClass<nalAnyPose> *a2, nalBaseSkeleton *a3);
};


extern bool Compatible(nalBaseSkeleton *a1, nalBaseSkeleton *a2);

template <typename T>
inline T *nalAnimPtrCast(nalAnimClass<nalAnyPose> *a1)
{
    if (a1 != nullptr && a1->m_vtbl == T::vtbl_ptr) {
        return bit_cast<T *>(a1);
    }

    return nullptr;
}

template <typename nalInstance>
inline void GetPose(nalInstance *inst, Float t, Float t_prev, typename nalInstance::nalPose &pose,
                    const typename nalInstance::nalPose &defaultPose)
{
    inst->GetPose(t, t_prev, pose, defaultPose);
}

extern void sub_826140(nalAnyPose &a1, Float a2, nalAnyPose &a3, nalAnyPose &a4);
