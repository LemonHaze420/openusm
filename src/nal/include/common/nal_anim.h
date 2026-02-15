#pragma once

#include "fixedstring.h"

#include <float.hpp>

#include <cstdint>

struct nalBaseSkeleton;
struct nalBasePose;

namespace nalComp {
struct nalCompSkeleton;
}

struct nalAnyPose {
    nalBasePose *field_0;

    nalAnyPose() = default;

    nalAnyPose(nalBaseSkeleton *a2);

    nalAnyPose(const nalBasePose *a2, bool a3);

    nalComp::nalCompSkeleton * GetSkeleton();

    void operator=(const nalAnyPose &a2);
};

template<typename T>
struct nalAnimClass {

    struct nalInstanceClass {
        std::intptr_t m_vtbl;
        float field_4;
        float field_8;
        nalBaseSkeleton *field_C;
        nalAnimClass<T> *field_10;

        nalInstanceClass(
            nalAnimClass<T> *a2,
            nalBaseSkeleton *a3);

        void * operator new(size_t size);

        void operator delete(void *ptr);

        //virtual
        ~nalInstanceClass();

        //0x00796D50
        void finalize(bool a2);
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

    auto *GetSkeleton() {
        return this->Skeleton;
    }

    void * CreateInstance(nalBaseSkeleton *skeleton);

    void _Release() {
    }

    //virtual
    void Release();

    //virtual
    void *VirtualCreateInstance(nalBaseSkeleton *Skel);

    static tlFixedString * get_string(nalAnimClass<T> *a1)
    {
        return &a1->field_8;
    }
};

struct nalBaseInstance : nalAnimClass<nalAnyPose>::nalInstanceClass {
    nalBaseInstance(
        nalAnimClass<nalAnyPose> *a2,
        nalBaseSkeleton *a3
    );

    //virtual
    void VirtualGetPose(
            Float a1,
            Float a2,
            nalBasePose *a3,
            const nalBasePose *a4);
};


extern bool Compatible(nalBaseSkeleton *a1, nalBaseSkeleton *a2);

template<typename T>
inline T *nalAnimPtrCast(nalAnimClass<nalAnyPose> *a1)
{
    if ( a1 != nullptr
        && a1->m_vtbl == T::vtbl_ptr )
    {
        return bit_cast<T *>(a1);
    }

    return nullptr;
}

extern void sub_826140(nalAnyPose &a1, Float a2, nalAnyPose &a3, nalAnyPose &a4);

