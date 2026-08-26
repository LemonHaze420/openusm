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

void ArmStdPoseDesc::BlendPoseDataPartial(ArmStdPoseDesc::StdPoseData *a1, uint32_t a2, Float a3,
                                          const ArmStdPoseDesc::StdPoseData *a4, const ArmStdPoseDesc::StdPoseData *a5,
                                          uint32_t a6) const
{
    TRACE("ArmStdPoseDesc::BlendPoseDataPartial");

    if constexpr (1) {
        for (int i = 0; i < 8; ++i) {
            if (((1 << i) & a6) != 0) {
                a1->field_0[i] = sub_5FD0C0(a3, a4->field_0[i], a5->field_0[i]);
            }
        }
    } else {
        void(__fastcall * func)(const void *,
                                void *edx,
                                ArmStdPoseDesc::StdPoseData *a1,
                                uint32_t a2,
                                Float a3,
                                const ArmStdPoseDesc::StdPoseData *a4,
                                const ArmStdPoseDesc::StdPoseData *a5,
                                uint32_t a6) = CAST(func, 0x005F70E0);
        func(this, nullptr, a1, a2, a3, a4, a5, a6);
    }
}

static constexpr float flt_891380 = 0.33000001f;

void ArmStdPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, unsigned int a2, const ArmStdPoseDesc::PerSkelData *a3,
                                       const ArmStdPoseDesc::StdPoseData *a4)
{
    TRACE("ArmStdPoseDesc::BuildBoneMatrices");

    if constexpr (0) {
        for (int i = 0; i < 8; ++i) {
            nalPositionOrientation a2a{a3->field_0[i], &a4->field_0[0][0]};
            nalMatrix4x4 v23{a2a};

            a1[a3->field_90[i]] = v23;
        }

        if (byte_959561) {
            auto v20 = sub_5F4960(a1[a3->field_90[3]], true);
            auto v21 = sub_5F4960(a1[a3->field_90[7]], false);
            auto v26 = v20 * flt_891380;
            a1[a3->field_B0] = sub_5F2FD0(v26, a3->field_60);
            a1[a3->field_B4] = sub_5F2FD0(v26, a3->field_6C);

            auto v27 = v21 * flt_891380;
            a1[a3->field_B8] = sub_5F2FD0(v27, a3->field_78);
            a1[a3->field_BC] = sub_5F2FD0(v27, a3->field_84);
        } else {
            a1[a3->field_B0] = sub_5F2FD0(0.0f, a3->field_60);
            a1[a3->field_B4] = sub_5F2FD0(0.0f, a3->field_6C);
            a1[a3->field_B8] = sub_5F2FD0(0.0f, a3->field_78);
            a1[a3->field_BC] = sub_5F2FD0(0.0f, a3->field_84);
        }

        auto *v14 = a3->field_90;
        for (int v13 = 0; v13 < 8; ++v13) {
            if (v13 != 0 && v13 != 4) {
                a1[v14[0]] = sub_5FE000(a1[v14[0]], a1[*(v14 - 1)]);
            } else {
                a1[v14[0]] = sub_5FE000(a1[v14[0]], a1[a3->field_C0]);
            }

            ++v14;
        }

        for (int i = 0; i < 4; ++i) {
            auto v16 = a3->field_90[i + 8];
            if ( v16 == -1 ) {
                continue;
            }

            int v17{};
            if ( i != 0 ) {
                if ( i == 2 ) {
                    a1[a3->field_90[10]] = sub_5FE000(a1[a3->field_90[10]], a1[a3->field_90[6]]);
                    continue;
                }

                v17 = a3->field_90[i + 7];
            } else {
                v16 = a3->field_90[8];
                v17 = a3->field_90[2];
            }

            a1[v16] = sub_5FE000(a1[v16], a1[v17]);
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *,
                                uint32_t,
                                const ArmStdPoseDesc::PerSkelData *a3,
                                const ArmStdPoseDesc::StdPoseData *a4) = CAST(func, 0x005F7160);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
