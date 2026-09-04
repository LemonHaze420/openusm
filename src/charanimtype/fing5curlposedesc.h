#pragma once

#include "nal_system.h"

struct Fing5CurlPoseDesc {
    struct StdPoseData {
        float field_0[4];
        int field_10;
        int field_14;
        float field_18[20];
        int field_68;
        int field_6C;
    };

    struct PerSkelData {
        nalVector3 field_0[20];
        int field_F0[28];
        int field_160;
        int field_164;
        int field_168[10];
        int field_190;
        int field_194;
        int field_198[18];
        int field_1E0;
        int field_1E4;
    };

    struct PerAnimData {};

    //0x005F88C0
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const PerSkelData *a3, const StdPoseData *a4);
};
