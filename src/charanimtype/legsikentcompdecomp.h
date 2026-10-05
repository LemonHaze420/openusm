#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct LegsIKEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::IK> {};
