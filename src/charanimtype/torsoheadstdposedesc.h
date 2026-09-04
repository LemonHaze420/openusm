#pragma once

#include "float.hpp"
#include "nal_system.h"
#include "vector3d.h"
#include "vector4d.h"

#include <cstdint>

struct nalMatrix4x4;

struct TorsoHeadStdPoseDesc {
    struct StdPoseData {
        vector4d field_0[5];
        vector4d field_50;
        nalVector3 field_60;
        int field_6C;
    };

    struct PerSkelData {
        float field_0[4];
        nalVector3 field_10;
        nalVector3 field_1C[5];
        int field_58;
        int field_5C[5];
        int field_70;
    };

    struct PerAnimData {
        int field_0;
        uint8_t field_4[4];
    };

    bool field_0;

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(TorsoHeadStdPoseDesc::StdPoseData *a1, uint32_t a2,
                               const TorsoHeadStdPoseDesc::StdPoseData *a3);

    void BlendPoseDataPartial(StdPoseData *a1, uint32_t a2, Float a3, const StdPoseData *a4, const StdPoseData *a5,
                              uint32_t a6) const;

    //0x005F6610
    void BuildBoneMatrices(nalMatrix4x4 *a2, uint32_t a3, const PerSkelData *a4, const StdPoseData *a5);
};
