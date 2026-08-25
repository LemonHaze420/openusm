#include "torsoheadstdposedesc.h"

#include "common.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(TorsoHeadStdPoseDesc::StdPoseData, 0x70u);

void TorsoHeadStdPoseDesc::CopyPoseDataToNothing(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t,
                                                 const TorsoHeadStdPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}

void TorsoHeadStdPoseDesc::BlendPoseDataPartial(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t a2, Float a3,
                                                const TorsoHeadStdPoseDesc::StdPoseData *a4,
                                                const TorsoHeadStdPoseDesc::StdPoseData *a5, uint32_t a6) const
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(const void *,
                                void *edx,
                                StdPoseData *,
                                uint32_t,
                                Float,
                                const StdPoseData *,
                                const StdPoseData *,
                                uint32_t) = CAST(func, 0x005F64D0);
        func(this, nullptr, a1, a2, a3, a4, a5, a6);
    }
}
