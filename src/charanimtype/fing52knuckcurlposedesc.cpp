#include "fing52knuckcurlposedesc.h"

#include "common.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(Fing52KnuckCurlPoseDesc::StdPoseData, 0xC0u);

void Fing52KnuckCurlPoseDesc::CopyPoseDataToNothing(Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t,
                                                    const Fing52KnuckCurlPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}

void Fing52KnuckCurlPoseDesc::BlendPoseDataPartial(Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t a2, Float a3,
                                                   const Fing52KnuckCurlPoseDesc::StdPoseData *a4,
                                                   const Fing52KnuckCurlPoseDesc::StdPoseData *a5, uint32_t a6) const
{
    if constexpr (0) {
    } else {
        void(__fastcall * func)(const void *,
                                void *edx,
                                StdPoseData *a1,
                                uint32_t a2,
                                Float a3,
                                const StdPoseData *a4,
                                const StdPoseData *a5,
                                uint32_t a6) = CAST(func, 0x005F7EC0);
        func(this, nullptr, a1, a2, a3, a4, a5, a6);
    }
}
