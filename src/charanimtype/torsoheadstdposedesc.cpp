#include "torsoheadstdposedesc.h"

#include "common.h"

#include <cstring>

VALIDATE_SIZE(TorsoHeadStdPoseDesc::StdPoseData, 0x70u);

void TorsoHeadStdPoseDesc::CopyPoseDataToNothing(
        TorsoHeadStdPoseDesc::StdPoseData *a1,
        uint32_t,
        const TorsoHeadStdPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}
