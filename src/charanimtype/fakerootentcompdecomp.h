#pragma once

#include "charcompressor.h"
#include "float.hpp"

namespace nalChar {
struct nalCharAnim;
}

template <typename T>
struct FakerootEntCompDecomp {
    struct PerInstData {
        float previousRotation[4];
        typename T::StdPoseData first;
        typename T::StdPoseData second;
        CharEntropyDecoder::CharChannelDecoder decoder;
        uint32_t count;
        int32_t frame;
        float previousPosition[3];
        float time;
        CharEntropyQuantConverter::EncTrackData *Tracks()
        {
            return reinterpret_cast<CharEntropyQuantConverter::EncTrackData *>(this + 1);
        }
    };
    void RetrievePoseFromInst(typename T::StdPoseData &pose, PerInstData *state, const typename T::PerAnimData *data);
    void GetPose(typename T::StdPoseData *pose, uint32_t index, Float time, Float previousTime,
                 const nalChar::nalCharAnim *anim, const typename T::PerSkelData *skel,
                 const typename T::PerAnimData *data, const void *stream, PerInstData *state, const T &descriptor);
};

extern void FakerootEntCompDecomp_patch();
