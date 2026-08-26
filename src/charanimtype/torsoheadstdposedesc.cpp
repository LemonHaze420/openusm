#include "torsoheadstdposedesc.h"

#include "common.h"
#include "nal_system.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(TorsoHeadStdPoseDesc::StdPoseData, 0x70u);
VALIDATE_OFFSET(TorsoHeadStdPoseDesc::PerSkelData, field_58, 0x58);

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

void TorsoHeadStdPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a2, uint32_t a3, const TorsoHeadStdPoseDesc::PerSkelData *a4,
                                             const TorsoHeadStdPoseDesc::StdPoseData *a5)
{
    TRACE("TorsoHeadStdPoseDesc::BuildBoneMatrices");

    if constexpr (1) {
        for (int i = 0; i < 5; ++i) {
            nalPositionOrientation a2a{a4->field_1C[i], &a5->field_0[i][0]};

            nalMatrix4x4 v31{a2a};
            a2[a4->field_5C[i]] = v31;
        }

        if (this->field_0) {
            nalPositionOrientation a2a{a4->field_10, &a4->field_0[0]};

            nalMatrix4x4 v31{a2a};
            a2[a4->field_70] = v31;
        }

        nalPositionOrientation a2a{a5->field_60, &a5->field_50[0]};

        nalMatrix4x4 v31{a2a};
        auto v23 = a4->field_58;
        a2[v23] = v31;

        auto *v24 = &a4->field_58;
        for (int i = 0; i < 5; ++i) {
            a2[v24[1]] = sub_5FE000(a2[v24[1]], a2[v24[0]]);
            ++v24;
        }

        if (this->field_0) {
            a2[a4->field_70] = sub_5FE000(a2[a4->field_70], a2[a4->field_5C[2]]);
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *a2,
                                uint32_t a3,
                                const TorsoHeadStdPoseDesc::PerSkelData *a4,
                                const TorsoHeadStdPoseDesc::StdPoseData *a5) = CAST(func, 0x005F6610);
        func(this, nullptr, a2, a3, a4, a5);
    }
}
