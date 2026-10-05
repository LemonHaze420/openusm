#include "charcompressor.h"

#include <cmath>
#include <cstring>

namespace {


uint32_t ReadBits(CharEntropyDecoder::CharChannelDecoder &decoder, unsigned count)
{
    const auto *p = static_cast<const uint8_t *>(decoder.field_0);
    unsigned bit = decoder.field_4;
    uint32_t result = 0;
    unsigned written = 0;
    while (written < count) {
        const unsigned take = count - written < 8 - bit ? count - written : 8 - bit;
        result |= uint32_t((*p >> bit) & ((1u << take) - 1)) << written;
        written += take;
        bit += take;
        if (bit == 8) {
            ++p;
            bit = 0;
        }
    }
    decoder.field_0 = p;
    decoder.field_4 = static_cast<uint8_t>(bit);
    return result;
}

int SignedBits(CharEntropyDecoder::CharChannelDecoder &decoder, unsigned count)
{
    const bool negative = ReadBits(decoder, 1) != 0;
    const int magnitude = static_cast<int>(ReadBits(decoder, count));
    return negative ? -magnitude : magnitude;
}

int Mantissa(unsigned bits)
{
    return (bits & 4) ? int(bits) : int(bits) - 7;
}
int Scaled(int value, unsigned shift)
{
    return static_cast<int>(uint32_t(value) << shift);
}


unsigned DecodeSymbol(CharEntropyDecoder::CharChannelDecoder &d, unsigned codec, int &value)
{
    if (codec >= 32 && codec < 47)
        codec -= 32;
    value = 0;
    auto bits = [&](unsigned n) {
        return ReadBits(d, n);
    };
    auto fixed = [&](unsigned n, unsigned run) {
        const unsigned v = bits(n);
        value = int(v) - int(1u << (n - 1));
        if (!v) {
            value = 0;
            return run;
        }
        return 1u;
    };
    switch (codec) {
    case 0:
    case 30:
    case 31:
    case 62:
    case 63:
        return 0;
    case 1:
        if (bits(1))
            value = int(bits(1)) * 2 - 1;
        return 1;
    case 2:
    case 4:
    case 8:
        if (!bits(1))
            return codec == 2 ? 7 : 4;
        if (!bits(1))
            return 2;
        if (bits(1)) {
            unsigned v = bits(codec == 8 ? 2 : 1);
            value = codec == 8 ? int(v + (v >> 1)) - 2 : int(v) * 2 - 1;
        }
        return 1;
    case 3:
        if (!bits(1))
            return 3;
        if (bits(1))
            value = int(bits(1)) * 2 - 1;
        return 1;
    case 5:
        return fixed(2, 6);
    case 6:
        if (bits(1)) {
            unsigned v = bits(2);
            value = int(v + (v >> 1)) - 2;
        }
        return 1;
    case 7: {
        unsigned v = bits(2);
        if (!v)
            v = bits(1) * 4;
        value = int(v) - 2;
        return 1;
    }
    case 9:
    case 11:
    case 50: {
        unsigned v = bits(2);
        if (v)
            value = int(v) - 2;
        else {
            unsigned n = codec == 9 ? 2 : codec == 11 ? 3 : 5;
            v = bits(n);
            value = int(v) - ((v & (1u << (n - 1))) ? int((1u << (n - 1)) - 2) : int((1u << (n - 1)) + 1));
        }
        return 1;
    }
    case 10:
        return fixed(3, 3);
    case 12:
        return fixed(4, 4);
    case 13:
    case 53: {
        const unsigned prefix = bits(3);
        if (prefix >= 3)
            value = int(prefix) - 5;
        else if (prefix == 2)
            value = bits(1) ? 3 : -3;
        else {
            value = int(bits(codec == 13 ? 2 : 5)) + 4;
            if ((codec == 13 && prefix == 1) || (codec == 53 && prefix == 0))
                value = -value;
        }
        return 1;
    }
    case 14:
    case 16: {
        unsigned v = bits(2);
        if (v)
            value = int(v) - 2;
        else if (!bits(1)) {
            v = bits(2);
            value = (v & 2) ? int(v) : int(v) - 3;
        } else {
            const unsigned shift = codec == 16 ? bits(1) : 0;
            value = Scaled(Mantissa(bits(3)), shift);
        }
        return 1;
    }
    case 15:
    case 19:
    case 22:
    case 23:
    case 24:
    case 26: {
        if (!bits(1))
            return fixed(codec == 23 || codec == 24 || codec == 26 ? 4 : 3,
                         codec == 23 || codec == 24 || codec == 26 ? 5 : 4);
        unsigned shift;
        if (codec == 15)
            shift = bits(1);
        else if (codec == 19 || codec == 23) {
            shift = bits(1) ? bits(1) + 1 : 0;
            if (codec == 23)
                ++shift;
        } else
            shift = bits(codec == 26 ? 3 : 2) + (codec == 22 ? 0 : 1);
        value = Scaled(Mantissa(bits(3)), shift);
        return 1;
    }
    case 17:
    case 21:
    case 49:
    case 52:
        if (!bits(1))
            return bits(1) ? 1 : 8;
        if (!bits(1)) {
            value = bits(1) ? 1 : -1;
            return 1;
        }
        if (codec == 49 || codec == 52) {
            unsigned n = codec == 49 ? 5 : 6;
            unsigned v = bits(n);
            value = int(v) - ((v & (1u << (n - 1))) ? int((1u << (n - 1)) - 2) : int((1u << (n - 1)) + 1));
        } else {
            unsigned exponent = bits(codec == 17 ? 1 : 2);
            if (!exponent) {
                unsigned v = bits(2);
                value = (v & 2) ? int(v) : int(v) - 3;
            } else {
                const unsigned shift = codec == 17 ? bits(1) : exponent - 1;
                value = Scaled(Mantissa(bits(3)), shift);
            }
        }
        return 1;
    case 18: {
        unsigned v = bits(5);
        if (!v)
            return 5;
        unsigned exponent = v & 3;
        value = exponent ? Scaled(Mantissa(v >> 2), exponent - 1) : int(v >> 2) - 4;
        return 1;
    }
    case 20: {
        unsigned prefix = bits(3);
        if (prefix >= 3)
            value = int(prefix) - 5;
        else if (!prefix)
            value = bits(1) ? 3 : -3;
        else {
            unsigned shift = prefix == 1 ? 0 : bits(1) + 1;
            value = Scaled(Mantissa(bits(3)), shift);
        }
        return 1;
    }
    case 25:
    case 27:
    case 28: {
        unsigned prefix = bits(3);
        if (prefix < 2) {
            unsigned magnitude = bits(2);
            if (!prefix && !magnitude)
                return 5;
            value = prefix ? int(magnitude) : -int(magnitude);
        } else {
            unsigned shift = prefix - 2;
            if (prefix == 7 && codec != 25)
                shift = bits(codec == 27 ? 3 : 4) + 5;
            value = Scaled(Mantissa(bits(3)), shift);
        }
        return 1;
    }
    case 29: {
        unsigned prefix = bits(5);
        if (prefix < 2) {
            value = int(bits(2)) + 1;
            if (!prefix)
                value = -value;
        } else if (prefix != 2)
            value = Scaled(Mantissa(bits(3)), prefix - 3);
        return 1;
    }
    case 47:
        return fixed(5, 5);
    case 48:
        if (!bits(1))
            return 4;
        if (bits(1)) {
            unsigned v = bits(5);
            value = int(v + (v >> 4)) - 16;
        }
        return 1;
    case 51:
        return fixed(6, 6);
    case 54:
        return fixed(7, 7);
    case 55:
    case 56:
    case 57:
    case 58:
        return fixed(codec - 47, 8);
    case 59:
        return fixed(16, 8);
    case 60:
        return fixed(24, 8);
    case 61: {
        uint32_t v = bits(32);
        if (v == 0x80000000u)
            return 8;
        value = static_cast<int32_t>(v);
        return 1;
    }
    }
    return 0;
}

using Track = CharEntropyQuantConverter::EncTrackData;
Track *Tracks(Track *data)
{
    return data;
}

void QuaternionStep(Track *track)
{
    float q[4], d[4];
    float qsum = 0, dsum = 0;
    for (int i = 0; i < 3; ++i) {
        q[i] = track[i].whole;
        d[i] = track[i].delta;
        qsum += q[i] * q[i];
        dsum += d[i] * d[i];
    }
    q[3] = std::sqrt(std::fabs(1.0f - qsum));
    d[3] = std::sqrt(std::fabs(1.0f - dsum));
    const float inverse = 1.0f / std::sqrt(dsum + d[3] * d[3]);
    for (float &v : d)
        v *= inverse;
    float result[4]{d[3] * q[0] + d[0] * q[3] + d[1] * q[2] - d[2] * q[1],
                    d[3] * q[1] + d[1] * q[3] + d[2] * q[0] - d[0] * q[2],
                    d[3] * q[2] + d[2] * q[3] + d[0] * q[1] - d[1] * q[0],
                    d[3] * q[3] - d[0] * q[0] - d[1] * q[1] - d[2] * q[2]};
    for (int i = 0; i < 3; ++i)
        track[i].whole = result[3] < 0 ? -result[i] : result[i];
}
}  // namespace

unsigned CharEntropyDecoder::DecodeChannel(CharChannelDecoder &decoder, unsigned codec, int &value)
{
    return DecodeSymbol(decoder, codec, value);
}

void CharEntropyQuantConverter::UnEntropyLinearTrack(EncTrackData *data, const uint8_t *, uint32_t index)
{
    auto &t = Tracks(data)[index];
    t.delta += t.second;
    t.whole += t.delta;
}
void CharEntropyQuantConverter::UnEntropyLinearTrackInitial(EncTrackData *data, const uint8_t *, uint32_t index)
{
    auto &t = Tracks(data)[index];
    t.whole += t.delta;
}
void CharEntropyQuantConverter::UnEntropyQuaternionTracks(EncTrackData *data, const uint8_t *, uint32_t index)
{
    auto *t = Tracks(data) + index;
    for (int i = 0; i < 3; ++i)
        t[i].delta += t[i].second;
    QuaternionStep(t);
}
void CharEntropyQuantConverter::UnEntropyQuaternionTracksInitial(EncTrackData *data, const uint8_t *, uint32_t index)
{
    QuaternionStep(Tracks(data) + index);
}
void CharEntropyQuantConverter::DecodeDequantTracks(EncTrackData *data, const uint8_t *codecs,
                                                    CharEntropyDecoder::CharChannelDecoder &decoder, uint32_t frame,
                                                    uint32_t start, uint32_t count, Float scale, bool scene)
{
    constexpr unsigned widths[2][4]{{2, 4, 7, 20}, {4, 7, 12, 30}};
    auto *tracks = Tracks(data);
    for (uint32_t i = start; i < start + count; ++i) {
        auto &t = tracks[i];
        if (frame == 0) {
            t.whole = SignedBits(decoder, widths[scene][3]) * (scale * 0.25f);
            t.zeros = 0;
        } else if (frame == 1) {
            t.delta = SignedBits(decoder, widths[scene][codecs[i] >> 6]) * scale;
        } else if ((codecs[i] & 63) && !t.zeros) {
            int value;
            t.zeros = DecodeChannel(decoder, codecs[i] & 63, value) - 1;
            t.second = value * scale;
        } else {
            if (codecs[i] & 63)
                --t.zeros;
            t.second = 0;
        }
    }
}
