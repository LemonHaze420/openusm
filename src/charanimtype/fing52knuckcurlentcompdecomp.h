#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct Fing52KnuckCurlEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::KnuckleCurl> {};
