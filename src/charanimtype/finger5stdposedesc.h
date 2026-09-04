#pragma once

#include "nal_system.h"
#include "vector4d.h"

struct Finger5StdPoseDesc {
    struct StdPoseData {
        vector4d field_0[30];
    };

    struct PerSkelData {
        nalVector3 field_0[30];
        int field_168[30];
        int field_1E0;
        int field_1E4;
    };

    struct PerAnimData {};

    //0x005F9430
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const PerSkelData *a3, const StdPoseData *a4);
};
