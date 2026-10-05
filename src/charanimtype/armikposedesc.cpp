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

void ArmIKPoseDesc::BuildBoneMatrices(nalMatrix4x4 *matrices, uint32_t, const PerSkelData *skel,
    const StdPoseData *pose)
{
    const auto &root = matrices[skel->field_F4];
    const auto inverseRoot = matrices[skel->field_F4].sub_5EC0A0();
    const auto *channels = reinterpret_cast<const float *>(pose);
    for (unsigned i = 0; i < 2; ++i) {
        auto &bone = matrices[skel->field_C0[i]];
        bone = nalMatrix4x4(nalPositionOrientation(skel->field_0[i], channels + 4*i));
        bone = sub_5FE000(bone, matrices[skel->field_F0]);
        bone = sub_5FE000(bone, inverseRoot);
    }
    for (unsigned i = 2; i < 4; ++i) {
        const nalVector3 position{{channels[16 + 3*(i-2)], channels[17 + 3*(i-2)], channels[18 + 3*(i-2)]}};
        matrices[skel->field_C0[i]] = nalMatrix4x4(nalPositionOrientation(position, channels + 4*i));
    }
    DecomposeIKSpin(matrices[skel->field_C0[4]], matrices[skel->field_C0[6]], matrices[skel->field_C0[0]],
        skel->field_30, matrices[skel->field_C0[2]], skel->field_90, LeftArmHeuristic, pose->field_58);
    DecomposeIKSpin(matrices[skel->field_C0[5]], matrices[skel->field_C0[7]], matrices[skel->field_C0[1]],
        skel->field_3C, matrices[skel->field_C0[3]], skel->field_A8, RightArmHeuristic, pose->field_5C);
    float leftTwist = 0.0f, rightTwist = 0.0f;
    if (byte_959561) {
        const auto left = sub_5FE000(matrices[skel->field_C0[2]], matrices[skel->field_C0[6]].sub_5EC0A0());
        const auto right = sub_5FE000(matrices[skel->field_C0[3]], matrices[skel->field_C0[7]].sub_5EC0A0());
        leftTwist = sub_5F4960(left, true) * 0.33000001f;
        rightTwist = sub_5F4960(right, false) * 0.33000001f;
    }
    matrices[skel->field_E0] = sub_5F2FD0(leftTwist, skel->field_60);
    matrices[skel->field_E4] = sub_5F2FD0(leftTwist, skel->field_6C);
    matrices[skel->field_E8] = sub_5F2FD0(rightTwist, skel->field_78);
    matrices[skel->field_EC] = sub_5F2FD0(rightTwist, skel->field_84);
    for (unsigned i = 0; i < 8; ++i)
        matrices[skel->field_C0[i]] = sub_5FE000(matrices[skel->field_C0[i]], root);
    const int twistBones[4]{skel->field_E0, skel->field_E4, skel->field_E8, skel->field_EC};
    for (unsigned i = 0; i < 4; ++i) {
        if (twistBones[i] == -1) continue;
        const int parent = i == 0 ? skel->field_C0[6] : i == 2 ? skel->field_C0[7] : twistBones[i - 1];
        matrices[twistBones[i]] = sub_5FE000(matrices[twistBones[i]], matrices[parent]);
    }
}
