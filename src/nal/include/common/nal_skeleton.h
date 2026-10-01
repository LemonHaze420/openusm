#pragma once

#include <cstdint>

#include <fixedstring.h>
#include <float.hpp>
#include <hashstring.h>

#include <tl_system.h>


struct nalBasePose;
struct nalMatrix4x4;
struct nalPositionOrientation;

struct nalBaseSkeleton {
    std::intptr_t m_vtbl;
    int Version;
    tlFixedString Name;
    tlFixedString AnimTypeName;
    int field_48;
    int field_4C;
    tlFileBuf field_50;

    /* virtual */ void Process();  // = 0;

    /* virtual */ void Release();  // = 0;

    /* virtual */ bool CheckVersion() const;  // = 0;

    //virtual
    void VirtualGetBoneMatrices(const nalBasePose *a1, nalMatrix4x4 *a2) const;  // = 0;

    //virtual
    void VirtualGetTrajectoryUpdate(const nalBasePose *a2,
                                    nalPositionOrientation *a3);  // = 0;

    //virtual
    nalBasePose *VirtualGetDefaultPose();  // = 0;

    //virtual
    nalBasePose *VirtualCreatePose() const;  // = 0;

    //virtual
    void VirtualDestroyPose(nalBasePose *a2);  // = 0;

    //virtual
    void VirtualCopyPose(nalBasePose &a2, const nalBasePose &a3) const;

    const tlFixedString &GetName() const
    {
        return this->Name;
    }

    const tlFixedString &GetAnimTypeName() const
    {
        return this->AnimTypeName;
    }

    static tlFixedString *get_string(nalBaseSkeleton *a1)
    {
        return bit_cast<tlFixedString *>(&a1->GetName());
    }
};

extern void sub_826190(nalBasePose &dst, Float a2, nalBasePose &src0, nalBasePose &src1);

//0x0078DC80
extern void *nalConstructSkeleton(void *a1);
