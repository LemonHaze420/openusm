#include "fing52knuckcurlposedesc.h"

#include "common.h"
#include "trace.h"
#include "utility.h"

#include <cstring>

VALIDATE_SIZE(Fing52KnuckCurlPoseDesc::StdPoseData, 0xC0u);

VALIDATE_OFFSET(Fing52KnuckCurlPoseDesc::PerSkelData, field_168, 0x168);
VALIDATE_OFFSET(Fing52KnuckCurlPoseDesc::PerSkelData, field_1E4, 0x1E4);

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

void Fing52KnuckCurlPoseDesc::BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2,
                                                const Fing52KnuckCurlPoseDesc::PerSkelData *a3,
                                                const Fing52KnuckCurlPoseDesc::StdPoseData *a4)
{
    TRACE("Fing52KnuckCurlPoseDesc::BuildBoneMatrices");

    if constexpr (1) {
        for (int v5 = 0; v5 < 20; ++v5) {
            if (a3->field_168[v5] != -1) {
                if (v5 >= 2) {
                    if (v5 >= 10) {
                        sub_5F42D0(a1[a3->field_168[v5]],
                                   a1[a3->field_168[v5 + 10]],
                                   a4->field_18[v5 + 8],
                                   a3->field_0[v5],
                                   a3->field_0[v5 + 10],
                                   (v5 % 10u) < 2);
                    } else {
                        a1[a3->field_168[v5]] = sub_5F4170(a4->field_18[v5], a4->field_18[v5 + 8], a3->field_0[v5]);
                    }
                } else {
                    nalPositionOrientation v21{a3->field_0[v5], &a4->field_0[v5 * 4]};

                    nalMatrix4x4 v22{v21};
                    a1[a3->field_168[v5]] = v22;
                }
            }
        }

        for (int v16 = 0; v16 < 10; ++v16) {
            if (a3->field_168[v16] != -1) {
                if (v16 != 0 && (v16 < 2 || v16 >= 6)) {
                    a1[a3->field_168[v16]] = sub_5FE000(a1[a3->field_168[v16]], a1[a3->field_1E4]);
                } else {
                    a1[a3->field_168[v16]] = sub_5FE000(a1[a3->field_168[v16]], a1[a3->field_1E0]);
                }

                a1[a3->field_168[v16 + 10]] = sub_5FE000(a1[a3->field_168[v16 + 10]], a1[a3->field_168[v16]]);
                a1[a3->field_168[v16 + 20]] = sub_5FE000(a1[a3->field_168[v16 + 20]], a1[a3->field_168[v16 + 10]]);
            }
        }
    } else {
        void(__fastcall * func)(void *,
                                void *edx,
                                nalMatrix4x4 *,
                                uint32_t,
                                const Fing52KnuckCurlPoseDesc::PerSkelData *,
                                const Fing52KnuckCurlPoseDesc::StdPoseData *) = CAST(func, 0x005F8230);
        func(this, nullptr, a1, a2, a3, a4);
    }
}
