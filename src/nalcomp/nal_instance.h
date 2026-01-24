#pragma once

#include "nal_anim.h"
#include "nal_system.h"

#include <float.hpp>

namespace nalComp {

struct nalCompAnim;
struct nalCompSkeleton;

struct nalCompInstance : nalBaseInstance {

    struct Internal {
        int field_0;
        int field_4;
        int field_8;
        void *field_C;
        bool field_10;
        bool field_11;
        char field_12;
        char field_13;
    };

    Internal *field_14;
    int field_18;
    void *field_1C;

    nalCompInstance(
        nalCompAnim *a2,
        nalCompSkeleton *a3);

    //0x00733D00
    void ConstructInstance();

    nalCompSkeleton * GetSkeleton();

    nalCompAnim * GetAnim();

    //virtual
    ~nalCompInstance();

    //virtual
    //0x00744BA0
    void VirtualGetPose(
            Float a1,
            Float a2,
            nalBasePose *a3,
            const nalBasePose *a4);

    void _BuildDirectMapping();

    //virtual
    //0x00736F70
    void BuildDirectMapping();

    void _BuildSkelRemapping();

    //virtual
    //0x0073E1A0
    void BuildSkelRemapping();

    void _BuildEmptyPoseArray();

    //virtual
    //0x0073E1A0
    void BuildEmptyPoseArray();

    void _BuildPerInstData();

    //virtual
    //0x00737130
    void BuildPerInstData();

    //0x00733EA0
    void GetPose(
            Float a2,
            Float a3,
            nalCompPose *a4,
            const nalCompPose *a5);

};

}

extern void nalCompInstance_patch();
