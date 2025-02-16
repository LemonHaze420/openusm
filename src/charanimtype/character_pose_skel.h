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

    void * operator new(size_t size);

    void operator delete(void *ptr);

    auto GetSkeleton() {
        return this->field_4;
    }

    void Blend(
        Float a2,
        nalCharPose *a3,
        nalCharPose *a4);

    void * GetNamedPoseData(CharComponentBase::Names a2);

    //virtual
    void InitializePoseDataFromSkel();
};

struct nalCharSkeleton : nalComp::nalCompSkeleton {
    int field_7C;
    nalCharPose *m_theDefaultPose;

    struct vtbl {
        void *Dummy;
        void *finalize;

        using Process_t = void (nalCharSkeleton::*)();
        Process_t Process;

        void *Release;

        using CheckVersion_t = bool (nalCharSkeleton::*)();
        CheckVersion_t CheckVersion;
    };

    int GetCompIxByName(CharComponentBase::Names a2) const;

    char * GetNamedPerSkelData(CharComponentBase::Names a2) const;

    nalCharPose * GetDefaultPose() const;

    nalCharPose * CreatePose() const;

    //virtual
    void Process();

    //virtual
    void Release();

    bool CheckVersion() {
        return this->Version == 0x10003;
    }

    //virtual
    const nalComp::nalCompSkeleton ** VirtualGetDefaultPose() const;

    //virtual
    const nalComp::nalCompSkeleton ** VirtualCreatePose() const;

    //0x005FCAC0
    void VirtualBlend(
        nalBasePose *a2,
        Float a3,
        nalBasePose *a4,
        nalBasePose *a5);

    static int vtbl_ptr;
};

} // nalChar

extern void nalChar_patch();
