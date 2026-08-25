#pragma once

#include <cstdint>

struct LegsStdPoseDesc {
    struct StdPoseData {
        char field_0[0x80];
    };

    struct PerAnimData {};

    struct PerSkelData {};

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(StdPoseData *a1, uint32_t, const StdPoseData *a3);
};
