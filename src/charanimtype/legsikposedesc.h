#pragma once

#include <cstdint>

struct LegsIKPoseDesc {
    struct StdPoseData {
        char field_0[0x60];
    };

    struct PerSkelData {
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(
        LegsIKPoseDesc::StdPoseData *a1,
        uint32_t,
        const LegsIKPoseDesc::StdPoseData *a3);
};
