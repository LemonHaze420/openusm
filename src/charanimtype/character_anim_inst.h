#pragma once

#include "nal_anim_comp.h"
#include "nal_instance.h"

#include "charcomponentbase.h"

struct nalBasePose;

namespace nalChar {

struct nalCharSkeleton;
struct nalCharInstance;

struct nalCharAnim : nalComp::nalCompAnim {

    nalCharAnim();

    //0x005FB5B0
    nalChar::nalCharInstance * CreateInstance(nalCharSkeleton *a1);

    //0x005EC3C0
    //virtual
    void _Process();

    //0x005EC3D0
    //virtual
    void _Release();

    //virtual
    bool _CheckVersion() const;

    //0x005FB5A0
    //virtual
    nalComp::nalCompInstance * _VirtualCreateInstance(nalBaseSkeleton *a1);

    auto * GetSkeleton() {
        return this->field_30;
    }

    void * GetPerAnimDataByName(CharComponentBase::Names a2);

    static int &vtbl_ptr;
};

struct nalCharInstance : nalComp::nalCompInstance {

    void finalize(bool);

    struct vtbl {
        void *finalize;
        void *VirtualGetPose;
        void *BuildDirectMapping;
        void *BuildSkelRemapping;
        void *BuildEmptyPoseArray;
        void *BuildPerInstData;
    };

    nalCharInstance(
        nalCharAnim *a2,
        nalCharSkeleton *a3);

    //virtual
    //0x005F1060
    ~nalCharInstance();

    //virtual
    void _VirtualGetPose(
        Float a1,
        Float a2,
        nalBasePose *a3,
        const nalBasePose *a4);

    //virtual
    //0x005F08A0
    void _BuildPerInstData();
};


} // namespace nalChar

extern void nalCharInstance_patch();
