#include "armstdposedesc.h"

#include "common.h"

#include <cstring>

VALIDATE_SIZE(ArmStdPoseDesc::StdPoseData, 0x80u);

void ArmStdPoseDesc::CopyPoseDataToNothing(ArmStdPoseDesc::StdPoseData *a1, uint32_t,
                                           const ArmStdPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(ArmStdPoseDesc::StdPoseData));
}
