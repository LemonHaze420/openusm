#include "legsstdposedesc.h"

#include "common.h"

#include <cstring>

VALIDATE_SIZE(LegsStdPoseDesc::StdPoseData, 0x80u);

void LegsStdPoseDesc::CopyPoseDataToNothing(
        LegsStdPoseDesc::StdPoseData *a1,
        uint32_t,
        const LegsStdPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}
