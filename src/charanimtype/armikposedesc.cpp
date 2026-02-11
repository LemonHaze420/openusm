#include "armikposedesc.h"

#include "common.h"

#include <cstring>

VALIDATE_SIZE(ArmIKPoseDesc::StdPoseData, 0x60u);

void ArmIKPoseDesc::CopyPoseDataToNothing(
        ArmIKPoseDesc::StdPoseData *a1,
        uint32_t,
        const ArmIKPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(ArmIKPoseDesc::StdPoseData));
}
