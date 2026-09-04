#pragma once

#include "float.hpp"
#include "nal_system.h"

#include <cstdint>

struct Fing52KnuckCurlPoseDesc {
    struct StdPoseData {
        float field_0[4];
        int field_10;
        int field_14;
        float field_18[20];
        int field_68;
        int field_6C[20];
        int field_BC;
    };

    struct PerSkelData {
        nalVector3 field_0[20];
        int field_F0[28];
        int field_160;
        int field_164;
        int field_168[20];
        int field_1B8;
        int field_1BC;
        int field_1C0;
        int field_1C4;
        int field_1C8;
        int field_1CC;
        int field_1D0;
        int field_1D4;
        int field_1D8;
        int field_1DC;
        int field_1E0;
        int field_1E4;
    };

    struct PerAnimData {
        int field_0;
        uint8_t field_4[4];
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}


    void CopyPoseDataToNothing(Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t,
                               const Fing52KnuckCurlPoseDesc::StdPoseData *a3);

    void BlendPoseDataPartial(Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t a2, Float a3,
                              const Fing52KnuckCurlPoseDesc::StdPoseData *a4,
                              const Fing52KnuckCurlPoseDesc::StdPoseData *a5, uint32_t a6) const;

    //0x005F8230
    void BuildBoneMatrices(nalMatrix4x4 *a1, uint32_t a2, const Fing52KnuckCurlPoseDesc::PerSkelData *a3,
                           const Fing52KnuckCurlPoseDesc::StdPoseData *a4);
};
