#pragma once

#include <cstdint>

struct TentaclesPoseDesc {
    struct StdPoseData {
        struct Channel {
            float diameter;
            float activity;
            float pull;
        };
        Channel channels[5];

        //0x005F0270
        float GetDiameterFromBone(uint32_t a2) const;

        //0x005F02A0
        float GetActivityFromBone(uint32_t a2) const;


        float GetPullFromBone(uint32_t a2) const;
    };

    struct PerSkelData {};
    struct PerAnimData {
        uint32_t mask;
        uint8_t codecs[1];
    };

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}

    void CopyPoseDataToNothing(TentaclesPoseDesc::StdPoseData *a1, uint32_t, const TentaclesPoseDesc::StdPoseData *a3);
};
