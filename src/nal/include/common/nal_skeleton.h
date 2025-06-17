#pragma once

#include <cstdint>

#include <fixedstring.h>
#include <float.hpp>

struct nalBasePose;

struct nalBaseSkeleton {
    std::intptr_t m_vtbl;
    int Version;
    tlFixedString field_8;
    tlFixedString field_28;

    //virtual
    nalBasePose* VirtualGetDefaultPose(); // = 0;

    //virtual
    nalBasePose * VirtualCreatePose(); // = 0;

    //virtual
    void VirtualDestroyPose(nalBasePose *a2); // = 0;

    const tlFixedString & GetName() const
    {
        return this->field_8;
    }

    const tlFixedString & GetAnimTypeName() const
    {
        return this->field_28;
    }

};

extern void sub_826190(nalBasePose &dst, Float a2, nalBasePose &src0, nalBasePose &src1);

//0x0078DC80
extern void *nalConstructSkeleton(void *a1);
