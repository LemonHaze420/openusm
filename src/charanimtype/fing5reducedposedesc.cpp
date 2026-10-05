#include "fing5reducedposedesc.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(Fing5ReducedPoseDesc::StdPoseData, 0xB0u);

VALIDATE_OFFSET(Fing5ReducedPoseDesc::PerSkelData, field_1E0, 0x1E0u);

void Fing5ReducedPoseDesc::BuildBoneMatrices(nalMatrix4x4 *matrices, uint32_t, const PerSkelData *skel,
    const StdPoseData *pose)
{
    const auto *channels = reinterpret_cast<const float *>(pose);
    for (unsigned i = 0; i < 30; ++i) {
        if (skel->field_168[i] == -1) continue;
        const unsigned finger = i % 15 < 3 ? unsigned(i >= 15) : i / 3 + unsigned(i < 15);
        const unsigned channel = finger + 10 * (i % 3);
        auto &bone = matrices[skel->field_168[i]];
        if (i % 3) sub_5F3080(bone, channels[channel+14], skel->field_0[channel]);
        else if (i == 0 || i == 15)
            bone = nalMatrix4x4(nalPositionOrientation(skel->field_0[channel], channels + 4*channel));
        else ReconstituteBaseKnuckle(bone, channels[channel+6], channels[channel+14], skel->field_0[channel]);
    }
    for (unsigned i = 0; i < 30; ++i) {
        if (skel->field_168[i] == -1) continue;
        const int parent = i % 3 ? skel->field_168[i-1] : i < 15 ? skel->field_1E0 : skel->field_1E4;
        matrices[skel->field_168[i]] = sub_5FE000(matrices[skel->field_168[i]], matrices[parent]);
    }
}
