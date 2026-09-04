#pragma once

#include "nal_anim.h"
#include "nal_system.h"

namespace nalCam {

struct nalCamInstance;
struct nalCamSkeleton;

struct nalCamAnim : nalAnimClass<nalAnyPose> {
    nalCamAnim();

    //0x005FCD80
    nalCamInstance *CreateInstance(nalCamSkeleton *a2);

    //virtual
    //0x005FCD70
    nalCamInstance *_VirtualCreateInstance(nalBaseSkeleton *a1);

    static int &vtbl_ptr;
};

struct nalCamBaseInstance : nalAnimClass<nalAnyPose>::nalInstanceClass {
    nalCamBaseInstance(nalAnimClass<nalAnyPose> *, nalBaseSkeleton *);
};

struct nalCamInstance : nalCamBaseInstance {
    nalCamInstance(nalCamAnim *a2, nalCamSkeleton *a3);
};
}  // namespace nalCam
