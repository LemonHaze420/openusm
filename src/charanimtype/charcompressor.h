#pragma once

#include "float.hpp"

#include <cstdint>

namespace CharEntropyDecoder {
struct CharChannelDecoder {
    const void *field_0;
    bool field_4;
    char field_5;
    uint16_t field_6;

    CharChannelDecoder(const void *a2, bool a3) : field_0(a2), field_4(a3), field_5(-1), field_6(0) {}
};
}  // namespace CharEntropyDecoder

struct CharEntropyQuantConverter {
    struct EncTrackData {
        float field_0[4][4];
    };

    static void UnEntropyLinearTrack(EncTrackData *a1, const uint8_t *a2, uint32_t a3);

    static void UnEntropyLinearTrackInitial(EncTrackData *a1, const uint8_t *, uint32_t a3);

    //0x005F29A0
    static void UnEntropyQuaternionTracks(EncTrackData *a1, const uint8_t *a2, uint32_t a3);

    //0x005F2C80
    static void UnEntropyQuaternionTracksInitial(EncTrackData *a1, const uint8_t *a2, uint32_t a3);

    //0x005F15D0
    static void DecodeDequantTracks(EncTrackData *a1, const uint8_t *a2, CharEntropyDecoder::CharChannelDecoder &a3,
                                    uint32_t a4, uint32_t a5, uint32_t a6, Float a7, bool a8);
};
