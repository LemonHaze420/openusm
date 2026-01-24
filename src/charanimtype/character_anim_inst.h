#pragma once

#include "nal_anim_comp.h"
#include "nal_instance.h"

#include "charcomponentbase.h"

struct nalBasePose;

namespace nalChar {

struct nalCharSkeleton;
struct nalCharInstance;

struct nalCharAnim : nalComp::nalCompAnim {

    //0x005FB5B0
    nalChar::nalCharInstance * CreateInstance(nalCharSkeleton *a1);

    struct vtbl {
        void *field_0;
        void *finalize;
        void *Process;
        void *Release;

        using CheckVersion_t = bool (nalCharAnim::*)();
        CheckVersion_t CheckVersion;

        vtbl(void *, void *, void *, void *, CheckVersion_t a4) : CheckVersion(a4) {}
    };

    //virtual
    bool CheckVersion() {
        return this->field_2C == 0x10003;
    }

    //virtual
    //0x005FB5A0
    nalComp::nalCompInstance * VirtualCreateInstance(nalBaseSkeleton *a1);

    auto * GetSkeleton() {
        return this->field_30;
    }

    void * GetPerAnimDataByName(CharComponentBase::Names a2);

    static int vtbl_ptr;
};

struct nalCharInstance : nalComp::nalCompInstance {

    void finalize(bool);

    void * operator new(size_t size);

    void operator delete(void *ptr);

    nalCharInstance(
        nalCharAnim *a2,
        nalCharSkeleton *a3);

    //virtual
    //0x005F1060
    ~nalCharInstance();

    //virtual
    void VirtualGetPose(
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
