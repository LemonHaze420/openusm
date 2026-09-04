#include "legsstdposedesc.h"

#include "common.h"
#include "trace.h"

#include <cstring>

VALIDATE_SIZE(LegsStdPoseDesc::StdPoseData, 0x80u);

void LegsStdPoseDesc::CopyPoseDataToNothing(LegsStdPoseDesc::StdPoseData *a1, uint32_t,
                                            const LegsStdPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(StdPoseData));
}

void LegsStdPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const LegsStdPoseDesc::PerSkelData *a3,
                                        const LegsStdPoseDesc::StdPoseData *a4)
{
    TRACE("LegsStdPoseDesc::BuildBoneMatrices");

    if constexpr (1) {
        for (int i = 0; i < 8; ++i) {
            nalPositionOrientation v18{a3->field_0[i], a4->field_0[i]};

            nalMatrix4x4 v19{v18};
            a1[a3->field_60[i]] = v19;
        }

        auto *v15 = &a3->field_60[0];
        for (int v14 = 0; v14 < 8; ++v14) {
            int v16 = 0;
            if (v14 == 0 || v14 == 4) {
                v16 = a3->field_80;
            } else {
                v16 = *(v15 - 1);
            }

            a1[v15[0]] = sub_5FE000(a1[v15[0]], a1[v16]);
            ++v14;
            ++v15;
        }

    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *,
                                uint32_t,
                                const LegsStdPoseDesc::PerSkelData *,
                                const LegsStdPoseDesc::StdPoseData *) = CAST(func, 0x005F6960);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
