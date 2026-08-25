#pragma once

#include "charcompressor.h"
#include "float.hpp"

#include <cstdint>

namespace nalChar {
struct nalCharAnim;
}

template <typename T>
struct LegsIKEntCompDecomp {
    struct PerInstData {
        T::StdPoseData field_0;
        T::StdPoseData field_60;
        CharEntropyDecoder::CharChannelDecoder field_C0;
        int field_C8;
        int field_CC;
        CharEntropyQuantConverter::EncTrackData field_D0;
    };

    void GetPose(T::StdPoseData *a1, unsigned int arg4, Float a3, Float a4, const nalChar::nalCharAnim *a6,
                 const T::PerSkelData *a7, const T::PerAnimData *a8, const void *a9, PerInstData *a2, const T &a11);

    //0x00600060
    void AdvanceAnimDataOneFrame(LegsIKEntCompDecomp<T>::PerInstData *a1, const T::PerAnimData *a2,
                                 const nalChar::nalCharAnim *a3, const uint8_t *a4, uint32_t a5);

    //0x005FFE50
    void RetrievePoseFromInst(T::StdPoseData &a1, LegsIKEntCompDecomp<T>::PerInstData *a2, const T::PerAnimData *a3);
};
