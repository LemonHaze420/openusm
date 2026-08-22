#pragma once

#include "float.hpp"

#include <cstdint>

struct LegsIKPoseDesc {
    struct PerAnimData {
        int field_0;
        uint8_t field_4[4];
    };

    struct StdPoseData {
        char field_0[0x60];
    };

    struct PerSkelData {};

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(LegsIKPoseDesc::StdPoseData *a1, uint32_t, const LegsIKPoseDesc::StdPoseData *a3);

    void BlendPoseDataPartial(StdPoseData *, uint32_t a1, Float a2, const StdPoseData *a3, const StdPoseData *a4,
                              uint32_t a5) const;
};
