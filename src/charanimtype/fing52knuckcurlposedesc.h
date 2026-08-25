#pragma once

#include "float.hpp"

#include <cstdint>

struct Fing52KnuckCurlPoseDesc {
    struct StdPoseData {
        char field_0[0xC0];
    };

    struct PerSkelData {};

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
};
