#include "charcompressor.h"

#include "common.h"
#include "utility.h"

void CharEntropyQuantConverter::UnEntropyLinearTrack(CharEntropyQuantConverter::EncTrackData *a1, const uint8_t *,
                                                     uint32_t a3)
{
    a1->field_0[a3][1] = a1->field_0[a3][1] + a1->field_0[a3][2];
    a1->field_0[a3][0] = a1->field_0[a3][0] + a1->field_0[a3][1];
}

void CharEntropyQuantConverter::UnEntropyLinearTrackInitial(CharEntropyQuantConverter::EncTrackData *a1,
                                                            const uint8_t *, uint32_t a3)
{
    a1->field_0[a3][0] = a1->field_0[a3][0] + a1->field_0[a3][1];
}

void CharEntropyQuantConverter::UnEntropyQuaternionTracks(CharEntropyQuantConverter::EncTrackData *a1,
                                                          const uint8_t *a2, uint32_t a3)
{
    if constexpr (0) {
    } else {
        void (*func)(EncTrackData *, const uint8_t *, uint32_t) = CAST(func, 0x005F29A0);
        func(a1, a2, a3);
    }
}

void CharEntropyQuantConverter::UnEntropyQuaternionTracksInitial(CharEntropyQuantConverter::EncTrackData *a1,
                                                                 const uint8_t *a2, uint32_t a3)
{
    if constexpr (0) {
    } else {
        void (*func)(EncTrackData *, const uint8_t *, uint32_t) = CAST(func, 0x005F2C80);
        func(a1, a2, a3);
    }
}

void CharEntropyQuantConverter::DecodeDequantTracks(CharEntropyQuantConverter::EncTrackData *a1, const uint8_t *a2,
                                                    CharEntropyDecoder::CharChannelDecoder &a3, uint32_t a4,
                                                    uint32_t a5, uint32_t a6, Float a7, bool a8)
{
    if constexpr (0) {
    } else {
        void (*func)(CharEntropyQuantConverter::EncTrackData *a1,
                     const uint8_t *a2,
                     CharEntropyDecoder::CharChannelDecoder &a3,
                     uint32_t a4,
                     uint32_t a5,
                     uint32_t a6,
                     Float a7,
                     bool a8) = CAST(func, 0x005F15D0);
        func(a1, a2, a3, a4, a5, a6, a7, a8);
    }
}
