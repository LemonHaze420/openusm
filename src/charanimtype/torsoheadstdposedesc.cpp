#include "torsoheadstdposedesc.h"

#include "common.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(TorsoHeadStdPoseDesc::StdPoseData, 0x70u);

void TorsoHeadStdPoseDesc::CopyPoseDataToNothing(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t,
                                                 const TorsoHeadStdPoseDesc::StdPoseData *a3)
{
    *a1 = *a3;
}

void TorsoHeadStdPoseDesc::BlendPoseDataPartial(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t a2, Float a3,
                                                const TorsoHeadStdPoseDesc::StdPoseData *a4,
                                                const TorsoHeadStdPoseDesc::StdPoseData *a5, uint32_t a6) const
{
    TRACE("TorsoHeadStdPoseDesc::BlendPoseDataPartial");

    if constexpr (1) {
        int v8 = 1;
        for (int i = 0; i < 5; ++i) {
            if ((v8 & a6) != 0) {
                a1->field_0[i] = sub_5FD0C0(a3, a4->field_0[i], a5->field_0[i]);
            }

            v8 *= 2;
        }

        if ((v8 & a6) != 0) {
            a1->field_50 = sub_5FD0C0(a3, a4->field_50, a5->field_50);
            auto v16 = (a5->field_60 - a4->field_60);
            auto v17 = v16 * a3;
            a1->field_60 = v17 + a4->field_60;
        }
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
