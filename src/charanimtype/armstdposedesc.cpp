#include "armstdposedesc.h"

#include "common.h"
#include "nal_math.h"
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
                a1->field_0[i] = math::Slerp(a3, a4->field_0[i], a5->field_0[i]);
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

void ArmStdPoseDesc::BuildBoneMatrices(nalMatrix4x4 *matrices, uint32_t, const PerSkelData *skel,
                                       const StdPoseData *pose)
{
    for (unsigned i = 0; i < 8; ++i)
        matrices[skel->field_90[i]] = nalMatrix4x4(nalPositionOrientation(skel->field_0[i], &pose->field_0[i][0]));

    const float leftTwist = byte_959561 ? sub_5F4960(matrices[skel->field_90[3]], true) * 0.33000001f : 0.0f;
    const float rightTwist = byte_959561 ? sub_5F4960(matrices[skel->field_90[7]], false) * 0.33000001f : 0.0f;
    matrices[skel->field_B0] = sub_5F2FD0(leftTwist, skel->field_60);
    matrices[skel->field_B4] = sub_5F2FD0(leftTwist, skel->field_6C);
    matrices[skel->field_B8] = sub_5F2FD0(rightTwist, skel->field_78);
    matrices[skel->field_BC] = sub_5F2FD0(rightTwist, skel->field_84);

    for (unsigned i = 0; i < 8; ++i) {
        const int parent = i == 0 || i == 4 ? skel->field_C0 : skel->field_90[i - 1];
        matrices[skel->field_90[i]] = sub_5FE000(matrices[skel->field_90[i]], matrices[parent]);
    }
    const int twistBones[4]{skel->field_B0, skel->field_B4, skel->field_B8, skel->field_BC};
    for (unsigned i = 0; i < 4; ++i) {
        if (twistBones[i] == -1)
            continue;
        const int parent = i == 0 ? skel->field_90[2] : i == 2 ? skel->field_90[6] : twistBones[i - 1];
        matrices[twistBones[i]] = sub_5FE000(matrices[twistBones[i]], matrices[parent]);
    }
}
