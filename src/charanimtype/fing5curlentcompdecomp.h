#pragma once

#include "nativeentcompdecomp.h"

template <typename T>
struct Fing5CurlEntCompDecomp : CharEntropyDecoder::PoseDecoder<T, CharEntropyDecoder::PoseChannels::Curl> {};
