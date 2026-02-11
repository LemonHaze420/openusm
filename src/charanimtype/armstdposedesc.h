#pragma once

#include <cstdint>

struct ArmStdPoseDesc {
    struct StdPoseData {
        char field_0[0x80];
    };

    struct PerSkelData {
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(
        ArmStdPoseDesc::StdPoseData *a1,
        uint32_t,
        const ArmStdPoseDesc::StdPoseData *a3);
};
