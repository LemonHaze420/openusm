#pragma once

#include <cstdint>

struct Fing52KnuckCurlPoseDesc {
    struct StdPoseData {
        char field_0[0xC0];
    };

    struct PerSkelData {};

    void SkelPoseProcess(uint32_t, PerSkelData *, StdPoseData *) {}


    void CopyPoseDataToNothing(Fing52KnuckCurlPoseDesc::StdPoseData *a1, uint32_t,
                               const Fing52KnuckCurlPoseDesc::StdPoseData *a3);
};
