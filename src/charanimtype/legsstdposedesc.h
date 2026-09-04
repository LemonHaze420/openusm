#pragma once

#include "nal_system.h"

#include <cstdint>

struct LegsStdPoseDesc {
    struct StdPoseData {
        float field_0[8][4];
    };

    struct PerAnimData {};

    struct PerSkelData {
        nalVector3 field_0[8];
        int field_60[8];
        int field_80;
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(StdPoseData *a1, uint32_t, const StdPoseData *a3);

    //0x005F6960
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const LegsStdPoseDesc::PerSkelData *a3,
                           const LegsStdPoseDesc::StdPoseData *a4);
};
