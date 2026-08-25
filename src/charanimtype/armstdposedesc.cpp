#include "armstdposedesc.h"

#include "common.h"
#include "trace.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(ArmStdPoseDesc::StdPoseData, 0x80u);

void ArmStdPoseDesc::CopyPoseDataToNothing(ArmStdPoseDesc::StdPoseData *a1, uint32_t,
                                           const ArmStdPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(ArmStdPoseDesc::StdPoseData));
}

void ArmStdPoseDesc::BlendPoseDataPartial(
        ArmStdPoseDesc::StdPoseData *a1,
        uint32_t a2,
        Float a3,
        const ArmStdPoseDesc::StdPoseData *a4,
        const ArmStdPoseDesc::StdPoseData *a5,
        uint32_t a6) const
{
    TRACE("ArmStdPoseDesc::BlendPoseDataPartial");

    if constexpr (0) {
    } else {
        void (__fastcall *func)(
                const void *, void *edx,
                ArmStdPoseDesc::StdPoseData *a1,
                uint32_t a2,
                Float a3,
                const ArmStdPoseDesc::StdPoseData *a4,
                const ArmStdPoseDesc::StdPoseData *a5,
                uint32_t a6) = CAST(func, 0x005F70E0);
        func(this, nullptr, a1, a2, a3, a4, a5, a6);
    }
}
