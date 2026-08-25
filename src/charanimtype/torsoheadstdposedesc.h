#pragma once

#include "float.hpp"

#include <cstdint>

struct TorsoHeadStdPoseDesc {
    struct StdPoseData {
        char field_0[0x70];
    };

    struct PerSkelData {};

    struct PerAnimData {
        int field_0;
        uint8_t field_4[4];
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t a2,
                               const TorsoHeadStdPoseDesc::StdPoseData *a3);

    void BlendPoseDataPartial(StdPoseData *a1, uint32_t a2, Float a3, const StdPoseData *a4, const StdPoseData *a5,
                              uint32_t a6) const;
};
