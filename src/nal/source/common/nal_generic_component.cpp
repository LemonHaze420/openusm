#include "nal_generic_component.h"
#include "charcompressor.h"
#include "common.h"
#include "nal_math.h"
#include "variables.h"
#include "vector4d.h"
#include <cmath>
#include <cstring>
#include <new>

namespace {
using Decoder = CharEntropyDecoder::CharChannelDecoder;
using Encoding = nalNativeStreamEncoding;
struct StreamCursor {
    nalGeneric::nalGenericAnim *anim;
    const nalGeneric::nalComponentInfo *info;
    void **skel;
    void **data;
    bool Active(int i) const
    {
        const unsigned bit = info->field_24 + i;
        return (anim->field_60[bit / 32] & (uint32_t(1) << (bit & 31))) != 0;
    }
};
template <typename T>
T *Align(T *p, unsigned alignment = 4)
{
    return reinterpret_cast<T *>((reinterpret_cast<std::uintptr_t>(p) + alignment - 1) &
                                 ~(std::uintptr_t(alignment) - 1));
}
void Advance(void **p, unsigned size)
{
    *p = static_cast<char *>(*p) + size;
}
void Advance(const void **p, unsigned size)
{
    *p = static_cast<const char *>(*p) + size;
}
unsigned Bits(Decoder &d, unsigned count)
{
    unsigned value = 0;
    auto *p = static_cast<const uint8_t *>(d.field_0);
    unsigned bit = d.field_4;
    for (unsigned consumed = 0; consumed < count;) {
        const unsigned available = 8 - bit;
        const unsigned take = count - consumed < available ? count - consumed : available;
        value |= unsigned((*p >> bit) & ((1u << take) - 1)) << consumed;
        consumed += take;
        bit += take;
        if (bit == 8) {
            ++p;
            bit = 0;
        }
    }
    d.field_0 = p;
    d.field_4 = uint8_t(bit);
    return value;
}
unsigned Peek(Decoder d, unsigned count)
{
    return Bits(d, count);
}
int Mantissa(unsigned value)
{
    return value & 4 ? int(value) : int(value) - 7;
}
int CompactDelta(Decoder &d, bool quaternion)
{
    const unsigned peek = Peek(d, 12);
    if (!(peek & 31)) {
        const unsigned bits = Bits(d, 12);
        return int(uint32_t(Mantissa(bits >> 9)) << (((bits >> 5) & 15) + (quaternion ? 9 : 17)));
    }
    if (!(peek & 1))
        return int((Bits(d, 5) >> 1) & 15) - 8;
    const unsigned bits = Bits(d, quaternion ? 7 : 8);
    return int(uint32_t(Mantissa((bits >> (quaternion ? 4 : 5)) & 7)) << (((bits >> 1) & (quaternion ? 7 : 15)) + 1));
}
int Magnitude(Decoder &d)
{
    static constexpr unsigned widths[]{3, 5, 8, 21};
    const unsigned value = Bits(d, widths[Bits(d, 2)]);
    return value & 1 ? -int(value >> 1) : int(value >> 1);
}
int ScalarInitial(Decoder &d)
{
    return Bits(d, 1) ? int(Bits(d, 32)) : int(Bits(d, 16)) - 0x8000;
}
int NextDelta(Decoder &d)
{
    const unsigned codec = uint8_t(d.field_5) & 63;
    if (codec == 0 || codec == 30 || codec == 31 || codec == 62 || codec == 63)
        return 0;
    if (d.field_6) {
        --d.field_6;
        return 0;
    }
    int value;
    const unsigned run = CharEntropyDecoder::DecodeChannel(d, codec, value);
    if (run > 1) {
        d.field_6 = uint16_t(run - 1);
        return 0;
    }
    return value;
}

struct ScalarState {
    Decoder decoder;
    int previous;
    int current;
};
struct QuatState {
    float q[4];
    Decoder decoder[3];
    int delta[3];
};
struct POState {
    QuatState quat;
    ScalarState position[3];
};
struct RLEState {
    const uint8_t *input;
    uint8_t remaining;
    uint8_t value;
    uint16_t padding;
};
static_assert(sizeof(ScalarState) == 16 && sizeof(QuatState) == 52 && sizeof(POState) == 100 && sizeof(RLEState) == 8);
void InitScalar(void *state, const void **input)
{
    const auto *p = static_cast<const uint8_t *>(*input);
    if (state)
        new (state) Decoder(p + 1, false);
    *input = p + 1 + *p;
}
void DecodeScalar(ScalarState &s, void *out, int stride, unsigned count, float scale)
{
    if (!count)
        return;
    auto emit = [&](int value) {
        *static_cast<float *>(out) = float(double(value) * std::fabs(scale));
        out = static_cast<char *>(out) + stride;
        --count;
    };
    if (s.decoder.field_5 == -1) {
        s.decoder.field_5 = char(Bits(s.decoder, 5) + (scale < 0 ? 32 : 0));
        s.previous = ScalarInitial(s.decoder);
        s.current =
            int(uint32_t(s.previous) + uint32_t(scale < 0 ? ScalarInitial(s.decoder) : CompactDelta(s.decoder, false)));
        emit(s.previous);
        if (!count) {
            s.decoder.field_5 = char(uint8_t(s.decoder.field_5) | 128);
            return;
        }
        emit(s.current);
    } else if (uint8_t(s.decoder.field_5) & 128) {
        emit(s.current);
        s.decoder.field_5 = char(uint8_t(s.decoder.field_5) & 127);
    }
    while (count) {
        const int next = int(uint32_t(NextDelta(s.decoder)) + 2u * uint32_t(s.current) - uint32_t(s.previous));
        s.previous = s.current;
        s.current = next;
        emit(next);
    }
}
void MultiplyQuat(float *out, const float *a, const float *b)
{
    const float q[]{a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1],
                    a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
                    a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3],
                    a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2]};
    std::memcpy(out, q, sizeof(q));
}
float Fourth(const float *q)
{
    const double x = q[0], y = q[1], z = q[2];
    return float(std::sqrt(std::fabs(1.0 - (x * x + y * y + z * z))));
}
void InitQuat(QuatState *s, const void *input, unsigned frames, float scale)
{
    if (!s)
        return;
    Decoder next(input, false);
    for (unsigned i = 0; i < 3; ++i) {
        auto &d = s->decoder[i];
        d = next;
        unsigned skip = 0;
        if (i < 2 && Bits(d, 1))
            skip = Bits(d, 11) + 256;
        d.field_5 = char(Bits(d, 5) + (scale < 0 ? 32 : 0));
        s->q[i] = std::fabs(scale) * 0.25f * Magnitude(d);
        s->delta[i] = scale < 0 ? Magnitude(d) : CompactDelta(d, true);
        next = d;
        if (skip) {
            const unsigned bit = next.field_4 + skip;
            next.field_0 = static_cast<const uint8_t *>(next.field_0) + bit / 8;
            next.field_4 = uint8_t(bit & 7);
        } else if (i < 2 && frames > 2) {
            for (unsigned frame = 2; frame < frames; ++frame)
                NextDelta(next);
        }
        next.field_5 = -1;
        next.field_6 = 0;
    }
    s->q[3] = Fourth(s->q);
    s->decoder[0].field_5 = char(uint8_t(s->decoder[0].field_5) | 128);
}
void DecodeQuat(QuatState &s, void *out, int stride, unsigned count, float scale, bool packed)
{
    scale = std::fabs(scale);
    while (count--) {
        const unsigned flags = uint8_t(s.decoder[0].field_5) & 192;
        if (flags != 128) {
            float dq[4];
            for (unsigned i = 0; i < 3; ++i) {
                if (!flags)
                    s.delta[i] = int(uint32_t(s.delta[i]) + uint32_t(NextDelta(s.decoder[i])));
                dq[i] = float(s.delta[i]) * scale;
            }
            dq[3] = Fourth(dq);
            MultiplyQuat(s.q, dq, s.q);
            if (!flags && !packed) {
                const float reciprocal =
                    1.0f / std::sqrt(s.q[0] * s.q[0] + s.q[1] * s.q[1] + s.q[2] * s.q[2] + s.q[3] * s.q[3]);
                for (float &v : s.q)
                    v *= reciprocal;
            }
        }
        if (flags)
            s.decoder[0].field_5 = char(uint8_t(s.decoder[0].field_5) - 64);
        if (packed) {
            auto *p = static_cast<int16_t *>(out);
            const float factor = s.q[3] < 0 ? -32767.0f : 32767.0f;
            for (unsigned i = 0; i < 3; ++i)
                p[i] = int16_t(s.q[i] * factor);
        } else
            std::memcpy(out, s.q, 16);
        out = static_cast<char *>(out) + stride;
    }
}
void Compose(float *out, const float *child, const float *parent)
{
    float result[7];
    MultiplyQuat(result, child, parent);
    const float x = parent[0], y = parent[1], z = parent[2], w = parent[3];
    const float tx = child[4], ty = child[5], tz = child[6];
    result[4] =
        (1 - 2 * y * y - 2 * z * z) * tx + (2 * x * y - 2 * z * w) * ty + (2 * x * z + 2 * y * w) * tz + parent[4];
    result[5] =
        (2 * x * y + 2 * z * w) * tx + (1 - 2 * x * x - 2 * z * z) * ty + (2 * y * z - 2 * x * w) * tz + parent[5];
    result[6] =
        (2 * x * z - 2 * y * w) * tx + (2 * y * z + 2 * x * w) * ty + (1 - 2 * x * x - 2 * y * y) * tz + parent[6];
    std::memcpy(out, result, sizeof(result));
}
void Inverse(float *out, const float *pose)
{
    float conjugate[]{-pose[0], -pose[1], -pose[2], pose[3], 0, 0, 0};
    const float translation[]{0, 0, 0, 1, -pose[4], -pose[5], -pose[6]};
    Compose(out, translation, conjugate);
}
void Power(float *out, const float *pose, unsigned exponent)
{
    const float identity[]{0, 0, 0, 1, 0, 0, 0};
    float base[7];
    std::memcpy(base, pose, 28);
    std::memcpy(out, identity, 28);
    while (exponent) {
        if (exponent & 1)
            Compose(out, out, base);
        exponent >>= 1;
        if (exponent)
            Compose(base, base, base);
    }
}

template <Encoding Kind>
struct NativeStream {
    static constexpr bool Byte = Kind == Encoding::RLE8 || Kind == Encoding::Signal || Kind == Encoding::Event;
    static constexpr bool PO =
        Kind == Encoding::PO || Kind == Encoding::EntropyPO || Kind == Encoding::EntropyTrajectoryPO;
    static constexpr bool Quat = Kind == Encoding::EntropyQuat || Kind == Encoding::Packed16EntropyQuat;
    static constexpr bool Morph = Kind == Encoding::MorphSlider;
    static constexpr bool Entropy = !Byte && Kind != Encoding::PO && !Morph;
    static constexpr bool Loop = Kind == Encoding::EntropyTrajectoryPO;
    static constexpr unsigned Size = Byte                                         ? 1
                                     : PO                                         ? 28
                                     : Quat                                       ? 16
                                     : (Kind == Encoding::EntropyFloat3 || Morph) ? 12
                                                                                  : 4;
    static constexpr unsigned DecodedSize = Kind == Encoding::Packed16EntropyQuat ? 6 : Size;
    static constexpr unsigned SkelSize = PO ? 8 : 4;
    static constexpr unsigned StateSize = Byte                              ? 8
                                          : (Kind == Encoding::PO || Morph) ? 4
                                          : PO                              ? 100
                                          : Quat                            ? 52
                                                                            : Size * 4;
    static constexpr unsigned DecodedAlignment = Kind == Encoding::Packed16EntropyQuat ? 2 : Byte ? 1 : 4;
    void *Type()
    {
        if constexpr (Morph)
            return &nalComponentMorphSliderBase::TypeID;
        else if constexpr (Byte)
            return &nalComponentU8Base::TypeID;
        else if constexpr (PO)
            return &nalComponentPOBase::TypeID;
        else if constexpr (Quat)
            return &nalComponentQuatBase::TypeID;
        else if constexpr (Size == 12)
            return &nalComponentFloat3Base::TypeID;
        else
            return &nalComponentFloat1Base::TypeID;
    }
    int ElementSize()
    {
        return Size;
    }
    void Blend(int count, void *out, const void *a, const void *b, float weight)
    {
        if constexpr (Morph) {
            std::memcpy(out, b, count * Size);
            return;
        }
        for (int i = 0; i < count; ++i) {
            auto *dst = static_cast<char *>(out) + i * Size;
            const auto *left = static_cast<const char *>(a) + i * Size;
            const auto *right = static_cast<const char *>(b) + i * Size;
            if constexpr (Byte) {
                if constexpr (Kind == Encoding::Event)
                    *dst = *right;
                else if constexpr (Kind == Encoding::Signal)
                    *dst = weight <= 0 ? *left : *right;
                else
                    *reinterpret_cast<uint8_t *>(dst) =
                        uint8_t((1 - weight) * uint8_t(*left) + weight * uint8_t(*right) + 0.5f);
            } else {
                unsigned scalar = 0;
                if constexpr (Quat || PO) {
                    *reinterpret_cast<vector4d *>(dst) = math::Slerp(
                        weight, *reinterpret_cast<const vector4d *>(left), *reinterpret_cast<const vector4d *>(right));
                    scalar = 4;
                }
                for (; scalar < Size / 4; ++scalar)
                    reinterpret_cast<float *>(dst)[scalar] =
                        (1 - weight) * reinterpret_cast<const float *>(left)[scalar] +
                        weight * reinterpret_cast<const float *>(right)[scalar];
            }
        }
    }
    void BlendArray(int count, void *out, const void *a, const void *b, const float **weights)
    {
        if constexpr (Morph)
            std::memcpy(out, b, count * Size);
        else if constexpr (Kind == Encoding::Event)
            std::memcpy(out, b, count * 4);
        else if constexpr (Kind == Encoding::Signal) {
            for (int i = 0; i < count; ++i)
                static_cast<float *>(out)[i] =
                    *(*weights)++ <= 0 ? static_cast<const float *>(a)[i] : static_cast<const float *>(b)[i];
        } else
            for (int i = 0; i < count; ++i)
                Blend(1,
                      static_cast<char *>(out) + i * Size,
                      static_cast<const char *>(a) + i * Size,
                      static_cast<const char *>(b) + i * Size,
                      *(*weights)++);
    }
    void Process(const nalGeneric::nalComponentInfo *info, void **skel, void **)
    {
        Lifecycle(info, skel);
    }
    void Lifecycle(const nalGeneric::nalComponentInfo *info, void **cursor)
    {
        if constexpr (!Byte && !Morph)
            *cursor = Align(*cursor);
        Advance(cursor, info->field_28 * Size);
    }
    void Copy(const nalGeneric::nalComponentInfo *info, void **out, const void **in)
    {
        if constexpr (!Byte) {
            *out = Align(*out);
            *in = Align(*in);
        }
        std::memcpy(*out, *in, info->field_28 * Size);
        Advance(out, info->field_28 * Size);
        Advance(in, info->field_28 * Size);
    }
    static unsigned PayloadSize(const void *data)
    {
        if constexpr (Kind == Encoding::Signal || Morph)
            return 4 + 4 * *static_cast<const uint32_t *>(data);
        else {
            const auto *p = static_cast<const uint8_t *>(data);
            const unsigned count = reinterpret_cast<const uint16_t *>(p)[1];
            unsigned size = 4;
            for (unsigned i = 0; i < count; ++i)
                size += 12 + 4 * p[size + 3];
            return size + 4;
        }
    }
    template <bool ReadScale = false>
    static float Begin(StreamCursor *cursor)
    {
        if constexpr (Entropy) {
            *cursor->skel = Align(*cursor->skel);
            *cursor->data = Align(*cursor->data);
            const float scale = ReadScale ? *static_cast<float *>(*cursor->data) : 1.0f;
            Advance(cursor->data, 4);
            return scale;
        } else if constexpr (Kind == Encoding::Signal || Kind == Encoding::Event || Morph)
            *cursor->data = Align(*cursor->data, Kind == Encoding::Event ? 2 : 4);
        return 1.0f;
    }
    static void TrackEnd(StreamCursor *cursor, bool active)
    {
        if constexpr (Entropy)
            Advance(cursor->skel, SkelSize);
        if constexpr (Loop) {
            if (active)
                Advance(cursor->data, 28);
        }
        if constexpr (Kind == Encoding::Signal || Kind == Encoding::Event || Morph) {
            if (active)
                Advance(cursor->data, PayloadSize(*cursor->data));
        }
    }
    void Setup(StreamCursor *cursor, void **state, const void **input, int frames)
    {
        *state = Align(*state);
        const float global = Begin<true>(cursor);
        for (int i = 0; i < cursor->info->field_28; ++i) {
            const bool active = cursor->Active(i);
            if (active) {
                if constexpr (Byte) {
                    *input = Align(*input);
                    const auto *p = static_cast<const uint8_t *>(*input);
                    if (*state) {
                        auto *s = static_cast<RLEState *>(*state);
                        s->input = p + 4;
                        s->remaining = 0;
                    }
                    *input = p + 4 + *reinterpret_cast<const uint32_t *>(p);
                } else if constexpr (Kind == Encoding::PO || Morph) {
                    *input = Align(*input);
                    if (*state)
                        *static_cast<const void **>(*state) = *input;
                    Advance(input, Size * frames);
                } else if constexpr (PO) {
                    for (unsigned channel = 0; channel < 3; ++channel)
                        InitScalar(static_cast<char *>(*state) + 52 + 16 * channel, input);
                    const auto *p = static_cast<const uint8_t *>(*input);
                    InitQuat(static_cast<QuatState *>(*state),
                             p + 1,
                             frames,
                             static_cast<float *>(*cursor->skel)[1] * global);
                    *input = p + 1 + *p;
                } else if constexpr (Quat) {
                    const auto *p = static_cast<const uint8_t *>(*input);
                    InitQuat(
                        static_cast<QuatState *>(*state), p + 1, frames, *static_cast<float *>(*cursor->skel) * global);
                    *input = p + 1 + *p;
                } else
                    for (unsigned channel = 0; channel < Size / 4; ++channel)
                        InitScalar(static_cast<char *>(*state) + 16 * channel, input);
                Advance(state, StateSize);
            }
            TrackEnd(cursor, active);
        }
    }
    void PartialDecode(StreamCursor *cursor, void **out, void **state, void *, int, int count, int stride)
    {
        *state = Align(*state);
        *out = Align(*out, DecodedAlignment);
        const float global = Begin<true>(cursor);
        for (int i = 0; i < cursor->info->field_28; ++i) {
            const bool active = cursor->Active(i);
            if (active) {
                if constexpr (Byte) {
                    auto &s = *static_cast<RLEState *>(*state);
                    auto *dst = static_cast<uint8_t *>(*out);
                    for (int frame = 0; frame < count; ++frame) {
                        if (!s.remaining) {
                            s.remaining = *s.input++;
                            s.value = *s.input++;
                        }
                        *dst = s.value;
                        --s.remaining;
                        dst += stride;
                    }
                } else if constexpr (Kind == Encoding::PO || Morph) {
                    auto **src = static_cast<const void **>(*state);
                    auto *dst = static_cast<char *>(*out);
                    for (int frame = 0; frame < count; ++frame) {
                        std::memcpy(dst, *src, Size);
                        dst += stride;
                        *src = static_cast<const char *>(*src) + Size;
                    }
                } else if constexpr (PO) {
                    auto &s = *static_cast<POState *>(*state);
                    const auto *scales = static_cast<const float *>(*cursor->skel);
                    for (unsigned j = 0; j < 3; ++j)
                        DecodeScalar(
                            s.position[j], static_cast<char *>(*out) + 16 + 4 * j, stride, count, scales[0] * global);
                    DecodeQuat(s.quat, *out, stride, count, scales[1] * global, false);
                } else if constexpr (Quat)
                    DecodeQuat(*static_cast<QuatState *>(*state),
                               *out,
                               stride,
                               count,
                               *static_cast<float *>(*cursor->skel) * global,
                               Kind == Encoding::Packed16EntropyQuat);
                else
                    for (unsigned j = 0; j < Size / 4; ++j)
                        DecodeScalar(static_cast<ScalarState *>(*state)[j],
                                     static_cast<char *>(*out) + 4 * j,
                                     stride,
                                     count,
                                     *static_cast<float *>(*cursor->skel) * global);
                Advance(state, StateSize);
                Advance(out, DecodedSize);
            }
            TrackEnd(cursor, active);
        }
    }
    void Decode(StreamCursor *cursor, void **out, const void **input, int count, int stride)
    {
        *out = Align(*out, DecodedAlignment);
        if constexpr (Entropy)
            Advance(cursor->data, 4);
        else if constexpr (Kind == Encoding::Signal || Kind == Encoding::Event || Morph)
            *cursor->data = Align(*cursor->data, Kind == Encoding::Event ? 2 : 4);
        for (int i = 0; i < cursor->info->field_28; ++i) {
            const bool active = cursor->Active(i);
            if (active) {
                if constexpr (Byte || Kind == Encoding::PO || Morph)
                    *input = Align(*input);
                if constexpr (Morph) {
                    auto *dst = static_cast<char *>(*out);
                    for (int frame = 0; frame < count; ++frame) {
                        std::memcpy(dst, *input, Size);
                        dst += stride;
                        Advance(input, Size);
                    }
                }
                Advance(out, DecodedSize);
            }
            TrackEnd(cursor, active);
        }
    }
    void Convert(StreamCursor *cursor, void *out, const void **decoded, const void *reference, const int *map)
    {
        *decoded = Align(*decoded, DecodedAlignment);
        Begin(cursor);
        for (int i = 0; i < cursor->info->field_28; ++i) {
            const bool active = cursor->Active(i);
            const int offset = map[cursor->info->field_24 + i];
            if (offset >= 0) {
                auto *dst = static_cast<char *>(out) + offset;
                if (active) {
                    if constexpr (Kind == Encoding::Packed16EntropyQuat) {
                        const auto *src = static_cast<const int16_t *>(*decoded);
                        auto *q = reinterpret_cast<float *>(dst);
                        for (unsigned j = 0; j < 3; ++j)
                            q[j] = float(src[j]) / 32768.0f;
                        q[3] = Fourth(q);
                    } else
                        std::memcpy(dst, *decoded, Size);
                } else
                    std::memcpy(dst, static_cast<const char *>(reference) + offset, Size);
            }
            if (active)
                Advance(decoded, DecodedSize);
            TrackEnd(cursor, active);
        }
    }
    void Cycle(StreamCursor *cursor, void *out, bool relative, const int *map)
    {
        if constexpr (Kind == Encoding::RLE8 || Kind == Encoding::PO)
            return;
        Begin(cursor);
        for (int i = 0; i < cursor->info->field_28; ++i) {
            const bool active = cursor->Active(i);
            if constexpr (Loop || Kind == Encoding::Signal || Kind == Encoding::Event) {
                const int offset = map[cursor->info->field_24 + i];
                if (active && offset >= 0) {
                    if constexpr (Loop) {
                        if (!relative)
                            std::memcpy(static_cast<char *>(out) + offset, *cursor->data, 28);
                    } else
                        static_cast<uint8_t *>(out)[offset] =
                            static_cast<const uint8_t *>(*cursor->data)[Kind == Encoding::Event ? 2 : 0];
                }
            }
            TrackEnd(cursor, active);
        }
    }
    void Trajectory(StreamCursor *cursor, void *out, const void *previous, int loops, bool relative, const int *map)
    {
        if constexpr (Kind == Encoding::RLE8 || Kind == Encoding::PO)
            return;
        Begin(cursor);
        for (int i = 0; i < cursor->info->field_28; ++i) {
            const bool active = cursor->Active(i);
            if constexpr (Loop || Kind == Encoding::Signal || Kind == Encoding::Event) {
                const int offset = map[cursor->info->field_24 + i];
                if (active && offset >= 0) {
                    if constexpr (Loop) {
                        if (!relative) {
                            auto *dst = reinterpret_cast<float *>(static_cast<char *>(out) + offset);
                            float inverse[7];
                            if (loops) {
                                float power[7];
                                Power(power,
                                      static_cast<float *>(*cursor->data),
                                      loops < 0 ? 0u - unsigned(loops) : unsigned(loops));
                                if (loops < 0) {
                                    Inverse(inverse, power);
                                    Compose(dst, dst, inverse);
                                } else
                                    Compose(dst, dst, power);
                            }
                            Inverse(inverse,
                                    reinterpret_cast<const float *>(static_cast<const char *>(previous) + offset));
                            Compose(dst, dst, inverse);
                        }
                    } else {
                        auto &dst = static_cast<uint8_t *>(out)[offset];
                        dst = uint8_t(dst +
                                      uint8_t(loops) *
                                          static_cast<const uint8_t *>(*cursor->data)[Kind == Encoding::Event ? 2 : 0] -
                                      static_cast<const uint8_t *>(previous)[offset]);
                    }
                }
            }
            TrackEnd(cursor, active);
        }
    }
    void ReleaseFrame(StreamCursor *cursor, void **out)
    {
        *out = Align(*out, DecodedAlignment);
        for (int i = 0; i < cursor->info->field_28; ++i)
            if (cursor->Active(i))
                Advance(out, DecodedSize);
    }
    void Empty(void **) {}
    void AlignCursor(void **cursor)
    {
        if constexpr (Entropy)
            *cursor = Align(*cursor);
    }
    void AlignAnimationCursor(void **cursor)
    {
        if constexpr (Entropy)
            *cursor = Align(*cursor);
    }
    void BeginAnimationCursor(void **cursor)
    {
        if constexpr (Entropy)
            Advance(cursor, 4);
    }
    void AlignTrackCursor(void **cursor)
    {
        if constexpr (Kind == Encoding::Signal || Kind == Encoding::Event || Morph || Loop)
            *cursor = Align(*cursor, Kind == Encoding::Event ? 2 : 4);
    }
    void EndTrackCursor(void **cursor)
    {
        if constexpr (Kind == Encoding::Signal || Kind == Encoding::Event || Morph)
            Advance(cursor, PayloadSize(*cursor));
        if constexpr (Loop)
            Advance(cursor, 28);
    }
    int CursorType()
    {
        return 1;
    }

    static std::intptr_t Table()
    {
        static void *table[]{func_address(&NativeStream::Type),
                             func_address(&NativeStream::ElementSize),
                             func_address(&NativeStream::Blend),
                             func_address(&NativeStream::BlendArray),
                             func_address(&NativeStream::Process),
                             func_address(&NativeStream::Setup),
                             func_address(&NativeStream::PartialDecode),
                             func_address(&NativeStream::Decode),
                             func_address(&NativeStream::Convert),
                             func_address(&NativeStream::Cycle),
                             func_address(&NativeStream::Trajectory),
                             func_address(&NativeStream::Lifecycle),
                             func_address(&NativeStream::Lifecycle),
                             func_address(&NativeStream::Copy),
                             func_address(&NativeStream::ReleaseFrame),
                             func_address(&NativeStream::Empty),
                             func_address(&NativeStream::Empty),
                             func_address(&NativeStream::AlignCursor),
                             func_address(&NativeStream::Empty),
                             func_address(&NativeStream::AlignAnimationCursor),
                             func_address(&NativeStream::BeginAnimationCursor),
                             func_address(&NativeStream::AlignTrackCursor),
                             func_address(&NativeStream::EndTrackCursor),
                             func_address(&NativeStream::CursorType)};
        return reinterpret_cast<std::intptr_t>(table);
    }
};
}  // namespace

void nalDecodeEntropyScalar(const void *input, void *output, unsigned frames, int stride, float scale)
{
    ScalarState state{Decoder(input, false), 0, 0};
    DecodeScalar(state, output, stride, frames, scale);
}

void nalDecodeEntropyQuaternion(const void *input, void *output, unsigned frames, int stride, float scale)
{
    QuatState state{{}, {Decoder(nullptr, false), Decoder(nullptr, false), Decoder(nullptr, false)}, {}};
    InitQuat(&state, input, frames, scale);
    DecodeQuat(state, output, stride, frames, scale, false);
}

std::intptr_t nalNativeStreamTable(nalNativeStreamEncoding encoding)
{
    static const std::intptr_t tables[]{NativeStream<Encoding::RLE8>::Table(),
                                        NativeStream<Encoding::Signal>::Table(),
                                        NativeStream<Encoding::Event>::Table(),
                                        NativeStream<Encoding::EntropyFloat1>::Table(),
                                        NativeStream<Encoding::EntropyFloat3>::Table(),
                                        NativeStream<Encoding::EntropyQuat>::Table(),
                                        NativeStream<Encoding::Packed16EntropyQuat>::Table(),
                                        NativeStream<Encoding::PO>::Table(),
                                        NativeStream<Encoding::EntropyPO>::Table(),
                                        NativeStream<Encoding::EntropyTrajectoryPO>::Table(),
                                        NativeStream<Encoding::MorphSlider>::Table()};
    return tables[static_cast<unsigned>(encoding)];
}

#if !STANDALONE_SYSTEM
nalComponentRLE8Int1 &Component_nalComponentRLE8Int1 = var<nalComponentRLE8Int1>(0x00946A90);
#else
nalComponentRLE8Int1 &Component_nalComponentRLE8Int1 = []() -> auto & {
    static nalComponentRLE8Int1 component;
    return component;
}();
#endif
nalComponentRLE8Int1::nalComponentRLE8Int1()
{
    m_vtbl = nalNativeStreamTable(Encoding::RLE8);
}
