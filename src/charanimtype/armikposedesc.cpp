#include "armikposedesc.h"

#include "common.h"
#include "trace.h"

#include <cstring>

VALIDATE_SIZE(ArmIKPoseDesc::StdPoseData, 0x60u);

VALIDATE_OFFSET(ArmIKPoseDesc::PerSkelData, field_C0, 0xC0u);

void ArmIKPoseDesc::CopyPoseDataToNothing(ArmIKPoseDesc::StdPoseData *a1, uint32_t,
                                          const ArmIKPoseDesc::StdPoseData *a3)
{
    std::memcpy(a1, a3, sizeof(ArmIKPoseDesc::StdPoseData));
}

static constexpr float flt_891380 = 0.33000001f;

void ArmIKPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a2, uint32_t a3, const ArmIKPoseDesc::PerSkelData *a4,
                                      const ArmIKPoseDesc::StdPoseData *a5)
{
    TRACE("ArmIKPoseDesc::BuildBoneMatrices");

    if constexpr (0) {
        auto &v35 = a2[a4->field_F4];
        auto v37 = v35.sub_5EC0A0();

        int i{0};
        for (; i < 2; ++i) {
            nalPositionOrientation v34{a4->field_0[i], &a5->field_0[i * 4]};

            nalMatrix4x4 v36{v34};
            a2[a4->field_C0[i]] = v36;

            a2[a4->field_C0[i]] = sub_5FE000(a2[a4->field_C0[i]], a2[a4->field_F0]);
            a2[a4->field_C0[i]] = sub_5FE000(a2[a4->field_C0[i]], v37);
        }

        for (; i < 4; ++i) {
            nalPositionOrientation v34{a5->field_28[i], &a5->field_0[i * 4]};

            nalMatrix4x4 v36{v34};
            a2[a4->field_C0[i]] = v36;
        }

        DecomposeIKSpin(a2[a4->field_C0[4]],
                        a2[a4->field_C0[6]],
                        a2[a4->field_C0[0]],
                        a4->field_30,
                        a2[a4->field_C0[2]],
                        a4->field_90,
                        LeftArmHeuristic,
                        a5->field_58);
        DecomposeIKSpin(a2[a4->field_C0[5]],
                        a2[a4->field_C0[7]],
                        a2[a4->field_C0[1]],
                        a4->field_3C,
                        a2[a4->field_C0[3]],
                        a4->field_A8,
                        RightArmHeuristic,
                        a5->field_5C);
        if (byte_959561) {
            nalMatrix4x4 v36{a2[a4->field_C0[2]]};
            auto v15 = a2[a4->field_C0[6]].sub_5EC0A0();
            v36 = sub_5FE000(v36, v15);
            auto v32 = sub_5F4960(v36, true);
            v36 = a2[a4->field_C0[3]];
            auto v16 = a2[a4->field_C0[7]].sub_5EC0A0();
            v36 = sub_5FE000(v36, v16);
            auto v33 = sub_5F4960(v36, false);
            auto v24 = v32 * flt_891380;
            a2[a4->field_E0] = sub_5F2FD0(v24, a4->field_60);
            a2[a4->field_E4] = sub_5F2FD0(v24, a4->field_6C);

            auto v25 = v33 * flt_891380;
            a2[a4->field_E8] = sub_5F2FD0(v25, a4->field_78);
            a2[a4->field_EC] = sub_5F2FD0(v25, a4->field_84);
        } else {
            a2[a4->field_E0] = sub_5F2FD0(0.0f, a4->field_60);
            a2[a4->field_E4] = sub_5F2FD0(0.0f, a4->field_6C);
            a2[a4->field_E8] = sub_5F2FD0(0.0f, a4->field_78);
            a2[a4->field_EC] = sub_5F2FD0(0.0f, a4->field_84);
        }

        for (int i = 0; i < 8; ++i) {
            a2[a4->field_C0[i]] = sub_5FE000(a2[a4->field_C0[i]], v35);
        }

        for (int i = 0; i < 4; ++i) {
            auto v20 = a4->field_C0[i + 8];
            if (v20 != -1) {
                int v21{};
                if (i != 0) {
                    if (i == 2) {
                        a2[a4->field_E8] = sub_5FE000(a2[a4->field_E8], a2[a4->field_C0[7]]);
                        continue;
                    }

                    v21 = a4->field_C0[i + 7];
                } else {
                    v20 = a4->field_E0;
                    v21 = a4->field_C0[6];
                }

                a2[v20] = sub_5FE000(a2[v20], a2[v21]);
            }
        }
    } else {
        void(__fastcall * func)(void *, void *edx, nalMatrix4x4 *, uint32_t, const PerSkelData *, const StdPoseData *) =
            CAST(func, 0x005F7760);
        func(this, nullptr, a2, a3, a4, a5);
    }
}
