#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct Fing5RedEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::Reduced> {};
