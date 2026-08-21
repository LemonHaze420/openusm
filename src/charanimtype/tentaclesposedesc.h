#pragma once

#include <cstdint>

struct TentaclesPoseDesc {
    struct StdPoseData {
        float field_0[1];
        float field_4[1];
        float field_8[1];
        int field_C[12];

        //0x005F0270
        float GetDiameterFromBone(uint32_t a2) const;

        //0x005F02A0
        float GetActivityFromBone(uint32_t a2) const;

        //0x
        float GetPullFromBone(uint32_t a2) const;
    };

    struct PerSkelData {};

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(TentaclesPoseDesc::StdPoseData *a1, uint32_t, const TentaclesPoseDesc::StdPoseData *a3);
};
