#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct TorsoHeadEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::Torso> {};
