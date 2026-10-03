#pragma once

#include "charcomponentbase.h"

#include <float.hpp>
#include <nal_pose_comp.h>
#include <nal_anim_comp.h>

struct nalBasePose;

namespace nalChar {

struct nalCharSkeleton;

struct nalCharPose : nalComp::nalCompPose {
    nalCharPose(const nalCharSkeleton *a2);

    nalCharPose(const nalChar::nalCharPose &a2, bool a3);

    ~nalCharPose();

    void *operator new(size_t size);

    void operator delete(void *ptr);

    auto GetSkeleton() const
    {
        return bit_cast<nalCharSkeleton *>(this->field_4);
    }

    void Blend(Float a2, const nalCharPose &a3, const nalCharPose &a4);

    void *GetNamedPoseData(CharComponentBase::Names a2);

    //virtual
    void InitializePoseDataFromSkel();
};

struct nalCharSkeleton : nalComp::nalCompSkeleton {
    int field_7C;
    nalCharPose *m_theDefaultPose;

    nalCharSkeleton();

    CharComponentBase *GetComponent(uint32_t a2)
    {
        return bit_cast<CharComponentBase *>(nalCompSkeleton::GetComponent(a2));
    }

    CharComponentBase *GetComponent(uint32_t a2) const
    {
        return bit_cast<CharComponentBase *>(nalCompSkeleton::GetComponent(a2));
    }

    int GetCompIxByName(CharComponentBase::Names a2) const;

    char *GetNamedPerSkelData(CharComponentBase::Names a2) const;

    char *GetCompPerSkelDataInt(int a2) const;

    char *GetCompDefaultPoseData(int iCompIx) const;

    nalCharPose *GetDefaultPose() const;

    nalCharPose *CreatePose() const;

    void DestroyPose(nalCharPose *a1);

    void CopyPose(nalCharPose *a1, const nalCharPose *a2);

    //virtual
    void _Process();

    //virtual
    void Release();

    bool _CheckVersion() const
    {
        return this->Version == 0x10003;
    }

    //0x00743820
    //virtual
    const nalComp::nalCompSkeleton **_VirtualGetDefaultPose() const;

    //0x005FE380
    //virtual
    const nalComp::nalCompSkeleton **_VirtualCreatePose() const;

    //0x005FCB10
    //virtual
    void _VirtualDestroyPose(nalBasePose *a2);

    //0x005FB560
    //virtual
    void _VirtualCopyPose(nalBasePose *a1, const nalBasePose *a2);

    //0x005FCAC0
    //virtual
    void _VirtualBlend(nalBasePose *a2, Float a3, nalBasePose *a4, nalBasePose *a5);

    static int &vtbl_ptr;
};

}  // namespace nalChar

extern void nalChar_patch();
