#include "nal_instance.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(nalComp::nalCompInstance, 0x20u);

void nalComp::nalCompInstance::VirtualGetPose(
        Float a1,
        Float a2,
        nalBasePose *a3,
        const nalBasePose *a4)
{
    const nalComp::nalCompPose *v5 = nullptr;
    if ( a4 != nullptr ) {
        v5 = (const nalComp::nalCompPose *)&a4[-1];
    }

    if ( a3 != nullptr ) {
        this->GetPose(a1, a2, (nalComp::nalCompPose *)&a3[-1], v5);
    } else {
        this->GetPose(a1, a2, nullptr, v5);
    }
}

void nalComp::nalCompInstance::GetPose(
        Float a2,
        Float a3,
        nalComp::nalCompPose *a4,
        const nalComp::nalCompPose *a5)
{
    THISCALL(0x00733EA0, this, a2, a3, a4, a5);
}
