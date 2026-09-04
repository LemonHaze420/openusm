#include "legsikposedesc.h"

#include "common.h"
#include "trace.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(LegsIKPoseDesc::StdPoseData, 0x60u);
VALIDATE_OFFSET(LegsIKPoseDesc::PerSkelData, field_60, 0x60);
VALIDATE_OFFSET(LegsIKPoseDesc::PerSkelData, field_90, 0x90);

void LegsIKPoseDesc::CopyPoseDataToNothing(LegsIKPoseDesc::StdPoseData *a1, uint32_t,
                                           const LegsIKPoseDesc::StdPoseData *a3)
{
    *a1 = *a3;
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

void LegsIKPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const LegsIKPoseDesc::PerSkelData *a3,
                                       const LegsIKPoseDesc::StdPoseData *a4)
{
    TRACE("LegsIKPoseDesc::BuildBoneMatrices");

    if constexpr (1) {
        for (int i = 2; i < 4; ++i) {
            nalPositionOrientation v26{a4->field_28[i], &a4->field_0[i][0]};

            nalMatrix4x4 v27{v26};
            a1[a3->field_90[i]] = v27;
        }

        DecomposeIKSpin(a1[a3->field_90[4]],
                        a1[a3->field_90[5]],
                        stru_9771C0,
                        a3->field_30,
                        a1[a3->field_90[2]],
                        a3->field_60,
                        LegHeuristic,
                        a4->field_58);
        DecomposeIKSpin(a1[a3->field_90[6]],
                        a1[a3->field_90[7]],
                        stru_9771C0,
                        a3->field_48,
                        a1[a3->field_90[3]],
                        a3->field_78,
                        LegHeuristic,
                        a4->field_5C);

        auto &v14 = a1[a3->field_B0];
        for (int i = 2; i < 8; ++i) {
            a1[a3->field_90[i]] = sub_5FE000(a1[a3->field_90[i]], v14);
        }

        for (int i = 0; i < 2; ++i) {
            nalPositionOrientation v26{a3->field_0[i], &a4->field_0[i][0]};

            nalMatrix4x4 v27{v26};
            a1[a3->field_90[i]] = v27;
            a1[a3->field_90[i]] = sub_5FE000(a1[a3->field_90[i]], a1[a3->field_90[i + 2]]);
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *,
                                uint32_t,
                                const LegsIKPoseDesc::PerSkelData *,
                                const LegsIKPoseDesc::StdPoseData *) = CAST(func, 0x005F6DA0);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
