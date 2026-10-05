#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct QuatsEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::Quaternions> {};
