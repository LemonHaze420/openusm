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
        nalAnimClass<nalAnyPose> *field_10;
    };

    std::intptr_t m_vtbl;
    nalAnimClass<nalAnyPose> *field_4;
    tlFixedString field_8;
    int field_28;
    int Version;
    nalBaseSkeleton *Skeleton;
    int field_34;
    int field_38;
    int InstanceCount;

    auto *GetSkeleton() {
        return this->Skeleton;
    }

    void * CreateInstance(nalBaseSkeleton *skeleton);

    //virtual
    void *VirtualCreateInstance(nalBaseSkeleton *Skel);
};

extern bool Compatible(nalBaseSkeleton *a1, nalBaseSkeleton *a2);

template<typename T>
inline T *nalAnimPtrCast(nalAnimClass<nalAnyPose> *a1)
{
    if ( a1 != nullptr
        && a1->m_vtbl == T::vtbl_ptr() )
    {
        return bit_cast<T *>(a1);
    }

    return nullptr;
}

extern void sub_826140(nalAnyPose &a1, Float a2, nalAnyPose &a3, nalAnyPose &a4);

