#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct FloatsEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::Floats> {};
