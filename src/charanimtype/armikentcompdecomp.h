#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct ArmIKEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::IK> {};
