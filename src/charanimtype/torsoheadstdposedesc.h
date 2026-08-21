#pragma once

#include <cstdint>

struct TorsoHeadStdPoseDesc {
    struct StdPoseData {
        char field_0[0x70];
    };

    struct PerSkelData {};

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t a2,
                               const TorsoHeadStdPoseDesc::StdPoseData *a3);
};
