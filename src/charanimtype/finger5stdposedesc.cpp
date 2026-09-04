#include "finger5stdposedesc.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(Finger5StdPoseDesc::StdPoseData, 0x1E0);

void Finger5StdPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const Finger5StdPoseDesc::PerSkelData *a3,
                                           const Finger5StdPoseDesc::StdPoseData *a4)
{
    TRACE("Finger5StdPoseDesc::BuildBoneMatrices");

    if constexpr (0) {
        for (int i = 0; i < 30; ++i) {
            if (a3->field_168[i] != -1) {
                nalPositionOrientation v19{a3->field_0[i], &a4->field_0[i][0]};

                nalMatrix4x4 v20{v19};
                a1[a3->field_168[i]] = v20;
            }
        }

        auto *v15 = a3->field_168;
        for (int v14 = 0; v14 < 30; ++v14) {
            if (a3->field_168[v14] != -1) {
                auto &v16 = a1[v15[0]];
                if (v14 % 3u) {
                    v16 = sub_5FE000(v16, a1[*(v15 - 1)]);
                } else if (v14 >= 15) {
                    v16 = sub_5FE000(v16, a1[a3->field_1E4]);
                } else {
                    v16 = sub_5FE000(v16, a1[a3->field_1E0]);
                }
            }

            ++v15;
        }
    } else {
        void(__fastcall * func)(void *, void *edx, nalMatrix4x4 *, uint32_t, const PerSkelData *, const StdPoseData *) =
            CAST(func, 0x005F9430);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
