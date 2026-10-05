#include "fing5curlposedesc.h"

#include "common.h"
#include "trace.h"

VALIDATE_SIZE(Fing5CurlPoseDesc::StdPoseData, 0x70u);

VALIDATE_OFFSET(Fing5CurlPoseDesc::PerSkelData, field_168, 0x168u);
VALIDATE_OFFSET(Fing5CurlPoseDesc::PerSkelData, field_1E0, 0x1E0u);

void Fing5CurlPoseDesc::BuildBoneMatrices(nalMatrix4x4 *matrices, uint32_t, const PerSkelData *skel,
                                          const StdPoseData *pose)
{
    const auto *channels = reinterpret_cast<const float *>(pose);
    for (unsigned i = 0; i < 10; ++i) {
        if (skel->field_168[i] == -1)
            continue;
        if (i < 2) {
            matrices[skel->field_168[i]] = nalMatrix4x4(nalPositionOrientation(skel->field_0[i], channels + 4 * i));
            Unconvert2Knuckle(matrices[skel->field_168[i + 10]],
                              matrices[skel->field_168[i + 20]],
                              channels[i + 16],
                              skel->field_0[i + 10],
                              skel->field_0[i + 20],
                              true);
        } else {
            ReconstituteFingerCurl(matrices[skel->field_168[i]],
                                   matrices[skel->field_168[i + 10]],
                                   matrices[skel->field_168[i + 20]],
                                   skel->field_0[i],
                                   skel->field_0[i + 10],
                                   skel->field_0[i + 20],
                                   channels[i + 16],
                                   channels[i + 6]);
        }
    }
    for (unsigned i = 0; i < 10; ++i) {
        if (skel->field_168[i] == -1)
            continue;
        const int parent = i != 0 && (i < 2 || i >= 6) ? skel->field_1E4 : skel->field_1E0;
        matrices[skel->field_168[i]] = sub_5FE000(matrices[skel->field_168[i]], matrices[parent]);
        matrices[skel->field_168[i + 10]] = sub_5FE000(matrices[skel->field_168[i + 10]], matrices[skel->field_168[i]]);
        matrices[skel->field_168[i + 20]] =
            sub_5FE000(matrices[skel->field_168[i + 20]], matrices[skel->field_168[i + 10]]);
    }
}
