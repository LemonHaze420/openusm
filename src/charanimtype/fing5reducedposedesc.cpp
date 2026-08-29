#include "fing5reducedposedesc.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(Fing5ReducedPoseDesc::StdPoseData, 0xB0u);

VALIDATE_OFFSET(Fing5ReducedPoseDesc::PerSkelData, field_1E0, 0x1E0u);

void Fing5ReducedPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const Fing5ReducedPoseDesc::PerSkelData *a3,
                                             const Fing5ReducedPoseDesc::StdPoseData *a4)
{
    TRACE("Fing5ReducedPoseDesc::BuildBoneMatrices");

    if constexpr (0) {
        for (int i = 0; i < 30; ++i) {
            if (a3->field_168[i] != -1) {
                int v8{};
                if ((i % 15) >= 3) {
                    v8 = i / 3u;
                    if (i < 15) {
                        ++v8;
                    }
                } else {
                    v8 = i >= 15;
                }

                auto v9 = v8 + 10 * (i % 3u);
                if (i % 3u) {
                    sub_5F3080(a1[a3->field_168[i]], a4->field_18[v9 + 8], a3->field_0[v9]);
                } else if (i != 15 && i != 0) {
                    ReconstituteBaseKnuckle(
                        a1[a3->field_168[i]], a4->field_18[v9], a4->field_18[v9 + 8], a3->field_0[v9]);
                } else {
                    nalPositionOrientation v22{a3->field_0[v9], &a4->field_0[4 * v9]};

                    nalMatrix4x4 v23{v22};
                    a1[a3->field_168[i]] = v23;
                }
            }
        }

        for (int i = 0; i < 30; ++i) {
            if (a3->field_168[i] != -1) {
                auto &v19 = a1[a3->field_168[i]];
                if (i % 3u != 0) {
                    v19 = sub_5FE000(v19, a1[i - 1]);
                } else if (i >= 15) {
                    v19 = sub_5FE000(v19, a1[a3->field_1E4]);
                } else {
                    v19 = sub_5FE000(v19, a1[a3->field_1E0]);
                }
            }
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *,
                                uint32_t,
                                const Fing5ReducedPoseDesc::PerSkelData *,
                                const Fing5ReducedPoseDesc::StdPoseData *) = CAST(func, 0x005F9150);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
