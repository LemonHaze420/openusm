#pragma once

#include "charcompressor.h"
#include "float.hpp"

#include <cstdint>

namespace nalChar {
struct nalCharAnim;
}

template <typename T>
struct QuatsEntCompDecomp {
    struct StdPoseData {};

    struct PerInstData {
        T::StdPoseData field_0;
        T::StdPoseData field_80;
        CharEntropyDecoder::CharChannelDecoder field_100;
        int field_108;
        int field_10C;
        CharEntropyQuantConverter::EncTrackData field_110;
    };

    void RetrievePoseFromInst(T::StdPoseData &a1, QuatsEntCompDecomp<T>::PerInstData *a2, const T::PerAnimData *a3);

    void AdvanceAnimDataOneFrame(QuatsEntCompDecomp<T>::PerInstData *a1, const nalChar::nalCharAnim *a2,
                                 const uint8_t *a3, uint32_t a4);

    void GetPose(T::StdPoseData *a1, uint32_t a2, Float a3, Float a4, const nalChar::nalCharAnim *a5,
                 const T::PerSkelData *a6, const T::PerAnimData *a7, const void *a8,
                 QuatsEntCompDecomp<T>::PerInstData *a9, const T &a10);
};
