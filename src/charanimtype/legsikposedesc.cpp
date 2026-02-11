#include "legsikposedesc.h"

#include "common.h"

#include <cstring>

VALIDATE_SIZE(LegsIKPoseDesc::StdPoseData, 0x60u);

void LegsIKPoseDesc::CopyPoseDataToNothing(
        LegsIKPoseDesc::StdPoseData *a1,
        uint32_t,
        const LegsIKPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}
