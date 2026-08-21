#include "fing52knuckcurlposedesc.h"

#include "common.h"

#include <cstring>

VALIDATE_SIZE(Fing52KnuckCurlPoseDesc::StdPoseData, 0xC0u);

void Fing52KnuckCurlPoseDesc::CopyPoseDataToNothing(Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t,
                                                    const Fing52KnuckCurlPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}
