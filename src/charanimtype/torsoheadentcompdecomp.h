#pragma once

#include "charcompressor.h"
#include "float.hpp"
#include "torsoheadstdposedesc.h"

namespace nalChar {
struct nalCharAnim;
}

template <typename T>
struct TorsoHeadEntCompDecomp {
    struct PerInstData {
        TorsoHeadStdPoseDesc::StdPoseData field_0;
        TorsoHeadStdPoseDesc::StdPoseData field_70;
        CharEntropyDecoder::CharChannelDecoder field_E0;
        int field_E8;
        int field_EC;
        CharEntropyQuantConverter::EncTrackData field_F0;
    };

    void RetrievePoseFromInst(T::StdPoseData &a1, TorsoHeadEntCompDecomp<T>::PerInstData *a2, const T::PerAnimData *a3);

    void AdvanceAnimDataOneFrame(TorsoHeadEntCompDecomp<T>::PerInstData *a1, const T::PerAnimData *a2,
                                 const nalChar::nalCharAnim *a3, const uint8_t *a4, uint32_t a5);

    void GetPose(T::StdPoseData *a2, uint32_t a3, Float a4, Float a5, const nalChar::nalCharAnim *a6,
                 const T::PerSkelData *a7, const T::PerAnimData *a8, const void *a9,
                 TorsoHeadEntCompDecomp<T>::PerInstData *a10, const T &a11);
};
