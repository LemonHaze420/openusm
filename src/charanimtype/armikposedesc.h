#pragma once

#include "nal_system.h"

#include <cstdint>

struct ArmIKPoseDesc {
    struct StdPoseData {
        float field_0[4];
        int field_10;
        int field_14;
        int field_18;
        int field_1C;
        int field_20;
        int field_24;
        nalVector3 field_28[4];
        float field_58;
        float field_5C;
    };

    struct PerSkelData {
        nalVector3 field_0[4];
        nalVector3 field_30;
        nalVector3 field_3C;
        int field_48[5];
        int field_5C;
        float field_60[3];
        float field_6C[3];
        float field_78[3];
        float field_84[3];
        IKSkelData field_90;
        int field_A4;
        IKSkelData field_A8;
        int field_BC;
        int field_C0[8];
        int field_E0;
        int field_E4;
        int field_E8;
        int field_EC;
        int field_F0;
        int field_F4;
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(StdPoseData *a1, uint32_t, const StdPoseData *a3);

    //0x005F7760
    void BuildBoneMatrices(nalMatrix4x4 *a2, uint32_t a3, const PerSkelData *a4, const StdPoseData *a5);
};
