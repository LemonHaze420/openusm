#include "fing5curlposedesc.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(Fing5CurlPoseDesc::StdPoseData, 0x70u);

VALIDATE_OFFSET(Fing5CurlPoseDesc::PerSkelData, field_168, 0x168u);
VALIDATE_OFFSET(Fing5CurlPoseDesc::PerSkelData, field_1E0, 0x1E0u);

void Fing5CurlPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const Fing5CurlPoseDesc::PerSkelData *a3,
                                          const Fing5CurlPoseDesc::StdPoseData *a4)
{
    TRACE("Fing5CurlPoseDesc::BuildBoneMatrices");

    if constexpr (0) {
        for (int i = 0; i < 10; ++i) {
            if (a3->field_168[i] != -1) {
                if (i >= 2) {
                    ReconstituteFingerCurl(a1[a3->field_168[i]],
                                           a1[a3->field_168[i + 10]],
                                           a1[a3->field_168[i + 20]],
                                           a3->field_0[i],
                                           a3->field_0[i + 10],
                                           a3->field_0[i + 20],
                                           a4->field_18[i + 10],
                                           a4->field_18[i]);
                } else {
                    nalPositionOrientation v30{a3->field_0[i], &a4->field_0[4 * i]};

                    nalMatrix4x4 v31{v30};
                    a1[a3->field_168[i]] = v31;
                    Unconvert2Knuckle(a1[a3->field_168[i + 10]],
                                      a1[a3->field_168[i + 20]],
                                      a4->field_18[i + 10],
                                      a3->field_0[i + 10],
                                      a3->field_0[i + 20],
                                      true);
                }
            }
        }

        for (int i = 0; i < 10; ++i) {
            if (a3->field_168[i] != -1) {
                if (i != 0 && (i < 2 || i >= 6)) {
                    a1[a3->field_168[i]] = sub_5FE000(a1[a3->field_168[i]], a1[a3->field_1E4]);
                } else {
                    a1[a3->field_168[i]] = sub_5FE000(a1[a3->field_168[i]], a1[a3->field_1E0]);
                }

                a1[a3->field_168[i + 10]] = sub_5FE000(a1[a3->field_168[i + 10]], a1[a3->field_168[i]]);
                a1[a3->field_168[i + 20]] = sub_5FE000(a1[a3->field_168[i + 20]], a1[a3->field_168[i + 10]]);
            }
        }

    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *,
                                uint32_t,
                                const Fing5CurlPoseDesc::PerSkelData *,
                                const Fing5CurlPoseDesc::StdPoseData *) = CAST(func, 0x005F88C0);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
