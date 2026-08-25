#pragma once

#include "charcompressor.h"
#include "float.hpp"

#include <cstdint>

namespace nalChar {
struct nalCharAnim;
}

template <typename T>
struct Fing52KnuckCurlEntCompDecomp {
    struct PerInstData {
        T::StdPoseData field_0;
        T::StdPoseData field_C0;
        CharEntropyDecoder::CharChannelDecoder field_180;
        int field_188;
        int field_18C;
    };

    void RetrievePoseFromInst(T::StdPoseData &a1, PerInstData *a2, const T::PerAnimData *a3);

    void AdvanceAnimDataOneFrame(PerInstData *a1, const T::PerAnimData *a2, const nalChar::nalCharAnim *a3,
                                 const uint8_t *a4, uint32_t a5);

    void GetPose(T::StdPoseData *a1, uint32_t a2, Float a3, Float a4, const nalChar::nalCharAnim *a5,
                 const T::PerSkelData *a6, const T::PerAnimData *a7, const void *a8, PerInstData *a9, const T &a10);
};
