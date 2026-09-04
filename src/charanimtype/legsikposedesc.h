#pragma once

#include "float.hpp"
#include "nal_system.h"
#include "vector3d.h"
#include "vector4d.h"

#include <cstdint>

struct LegsIKPoseDesc {
    struct PerAnimData {
        int field_0;
        uint8_t field_4[4];
    };

    struct StdPoseData {
        vector4d field_0[2];
        int field_20;
        int field_24;
        nalVector3 field_28[4];
        float field_58;
        float field_5C;
    };

    struct PerSkelData {
        nalVector3 field_0[2];
        int field_18;
        int field_1C;
        int field_20;
        int field_24;
        int field_28;
        int field_2C;
        nalVector3 field_30;
        int field_3C;
        int field_40;
        int field_44;
        nalVector3 field_48;
        int field_54;
        int field_58;
        int field_5C;
        IKSkelData field_60;
        int field_74;
        IKSkelData field_78;
        int field_8C;
        int field_90[8];
        int field_B0;
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(LegsIKPoseDesc::StdPoseData *a1, uint32_t, const LegsIKPoseDesc::StdPoseData *a3);

    void BlendPoseDataPartial(StdPoseData *, uint32_t a1, Float a2, const StdPoseData *a3, const StdPoseData *a4,
                              uint32_t a5) const;

    //0x005F6DA0
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const PerSkelData *a3, const StdPoseData *a4);
};
