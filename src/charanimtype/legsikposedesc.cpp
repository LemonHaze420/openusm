#include "legsikposedesc.h"

#include "common.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(LegsIKPoseDesc::StdPoseData, 0x60u);

void LegsIKPoseDesc::CopyPoseDataToNothing(LegsIKPoseDesc::StdPoseData *a1, uint32_t,
                                           const LegsIKPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}

void LegsIKPoseDesc::BlendPoseDataPartial(LegsIKPoseDesc::StdPoseData *a1, uint32_t a2, Float a3,
                                          const LegsIKPoseDesc::StdPoseData *a4, const LegsIKPoseDesc::StdPoseData *a5,
                                          uint32_t a6) const
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(const void *,
                                void *edx,
                                LegsIKPoseDesc::StdPoseData *,
                                uint32_t,
                                Float,
                                const LegsIKPoseDesc::StdPoseData *,
                                const LegsIKPoseDesc::StdPoseData *,
                                uint32_t) = CAST(func, 0x005F6C00);
        func(this, nullptr, a1, a2, a3, a4, a5, a6);
    }
}
