#include "finger5stdposedesc.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(Finger5StdPoseDesc::StdPoseData, 0x1E0);

void Finger5StdPoseDesc::BuildBoneMatrices(nalMatrix4x4 *matrices, uint32_t, const PerSkelData *skel,
    const StdPoseData *pose)
{
    for (unsigned i = 0; i < 30; ++i) {
        if (skel->field_168[i] == -1) continue;
        matrices[skel->field_168[i]] = nalMatrix4x4(nalPositionOrientation(skel->field_0[i], &pose->field_0[i][0]));
    }
    for (unsigned i = 0; i < 30; ++i) {
        if (skel->field_168[i] == -1) continue;
        const int parent = i % 3 ? skel->field_168[i-1] : i < 15 ? skel->field_1E0 : skel->field_1E4;
        matrices[skel->field_168[i]] = sub_5FE000(matrices[skel->field_168[i]], matrices[parent]);
    }
}
