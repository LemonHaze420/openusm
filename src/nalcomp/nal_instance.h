#pragma once

#include "nal_anim.h"
#include "nal_system.h"

#include <float.hpp>

namespace nalComp {

struct nalCompInstance : nalAnimClass<nalAnyPose>::nalInstanceClass {


    //virtual
    //0x00744BA0
    void VirtualGetPose(
            Float a1,
            Float a2,
            nalBasePose *a3,
            const nalBasePose *a4);


    //0x00733EA0
    void GetPose(
            Float a2,
            Float a3,
            nalCompPose *a4,
            const nalCompPose *a5);

};

}
