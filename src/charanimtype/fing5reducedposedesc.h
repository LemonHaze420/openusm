#pragma once

#include "nal_system.h"

#include <cstdint>

struct Fing5ReducedPoseDesc {
    struct PerSkelData {
        nalVector3 field_0[30];
        int field_168[30];
        int field_1E0;
        int field_1E4;
    };

    struct StdPoseData {
        float field_0[4];
        int field_10;
        int field_14;
        float field_18[30];
        int field_90;
        int field_94;
        int field_98;
        int field_9C;
        int field_A0;
        int field_A4;
        int field_A8;
        int field_AC;
    };

    struct PerAnimData {};

    //0x005F9150
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const Fing5ReducedPoseDesc::PerSkelData *a3,
                           const Fing5ReducedPoseDesc::StdPoseData *a4);
};
