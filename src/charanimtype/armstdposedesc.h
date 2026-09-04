#pragma once

#include "float.hpp"
#include "nal_system.h"
#include "vector4d.h"

#include <cstdint>

struct nalMatrix4x4;

struct ArmStdPoseDesc {
    struct StdPoseData {
        vector4d field_0[8];
    };

    struct PerSkelData {
        nalVector3 field_0[8];
        float field_60[3];
        float field_6C[3];
        float field_78[3];
        float field_84[3];
        int field_90[8];
        int field_B0;
        int field_B4;
        int field_B8;
        int field_BC;
        int field_C0;
    };

    struct PerAnimData {
        int field_0;
        uint8_t field_4[4];
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(ArmStdPoseDesc::StdPoseData *a1, uint32_t, const ArmStdPoseDesc::StdPoseData *a3);

    void BlendPoseDataPartial(StdPoseData *a1, uint32_t a2, Float a3, const StdPoseData *a4, const StdPoseData *a5,
                              uint32_t a6) const;

    //0x005F7160
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const PerSkelData *a3, const StdPoseData *a4);
};
