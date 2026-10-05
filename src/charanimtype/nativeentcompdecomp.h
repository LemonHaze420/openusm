#pragma once

#include "charcompressor.h"
#include "character_anim_inst.h"
#include "nal_math.h"

#include <cmath>
#include <cstring>

namespace CharEntropyDecoder {
enum class PoseChannels { Quaternions, Torso, IK, Floats, KnuckleCurl, Curl, Reduced };

inline void RetrieveQuaternion(float *out, const CharEntropyQuantConverter::EncTrackData *tracks)
{
    const float x = tracks[0].whole, y = tracks[1].whole, z = tracks[2].whole;
    out[0] = x;
    out[1] = y;
    out[2] = z;
    out[3] = std::sqrt(std::fabs(1.0f - (x * x + y * y + z * z)));
}


template <PoseChannels Kind, unsigned QuatCount, typename Visitor>
void VisitChannels(uint32_t mask, Visitor &&visit)
{
    constexpr unsigned count = Kind == PoseChannels::Quaternions   ? QuatCount
                               : Kind == PoseChannels::Torso       ? 6
                               : Kind == PoseChannels::IK          ? 4
                               : Kind == PoseChannels::Floats      ? 15
                               : Kind == PoseChannels::KnuckleCurl ? 20
                               : Kind == PoseChannels::Curl        ? 10
                                                                   : 30;
    for (unsigned i = 0; i < count; ++i) {
        if (!(mask & (1u << i)))
            continue;
        if constexpr (Kind == PoseChannels::Quaternions)
            visit(i * 4, true);
        else if constexpr (Kind == PoseChannels::Torso) {
            visit(i * 4, true);
            if (i == 5)
                for (unsigned j = 24; j < 27; ++j)
                    visit(j, false);
        } else if constexpr (Kind == PoseChannels::IK) {
            visit(i * 4, true);
            if (i >= 2) {
                for (unsigned j = 0; j < 3; ++j)
                    visit(16 + 3 * (i - 2) + j, false);
                visit(22 + i - 2, false);
            }
        } else if constexpr (Kind == PoseChannels::Floats)
            visit(i, false);
        else if constexpr (Kind == PoseChannels::Curl) {
            if (i < 2)
                visit(i * 4, true);
            else
                visit(i + 6, false);
            visit(i + 16, false);
        } else {
            if (i < 2)
                visit(i * 4, true);
            else {
                if (i < 10)
                    visit(i + 6, false);
                visit(i + 14, false);
            }
        }
    }
}

template <typename T, PoseChannels Kind>
struct PoseDecoder {
    using Pose = typename T::StdPoseData;
    struct PerInstData {
        Pose first;
        Pose second;
        CharChannelDecoder decoder;
        uint32_t count;
        int32_t frame;
        CharEntropyQuantConverter::EncTrackData *Tracks()
        {
            return reinterpret_cast<CharEntropyQuantConverter::EncTrackData *>(this + 1);
        }
    };
    static constexpr unsigned QuatCount = sizeof(Pose) / 16;
    static uint32_t Mask(const typename T::PerAnimData *data)
    {
        return *reinterpret_cast<const uint32_t *>(data);
    }
    static void AdvanceAnimDataOneFrame(PerInstData *state, const typename T::PerAnimData *data,
                                        const nalChar::nalCharAnim *anim, const uint8_t *codecs, uint32_t frame)
    {
        using Converter = CharEntropyQuantConverter;
        auto *tracks = state->Tracks();
        Converter::DecodeDequantTracks(tracks,
                                       codecs,
                                       state->decoder,
                                       frame,
                                       0,
                                       state->count,
                                       anim->GetAnimQuantScale() * (1.0f / 1024.0f),
                                       anim->IsSceneAnim());
        if (!frame)
            return;
        unsigned index = 0;
        VisitChannels<Kind, QuatCount>(Mask(data), [&](unsigned, bool quaternion) {
            if (quaternion) {
                if (frame == 1)
                    Converter::UnEntropyQuaternionTracksInitial(tracks, codecs, index);
                else
                    Converter::UnEntropyQuaternionTracks(tracks, codecs, index);
                index += 3;
            } else {
                if (frame == 1)
                    Converter::UnEntropyLinearTrackInitial(tracks, codecs, index);
                else
                    Converter::UnEntropyLinearTrack(tracks, codecs, index);
                ++index;
            }
        });
    }
    static void RetrievePoseFromInst(Pose &pose, PerInstData *state, const typename T::PerAnimData *data)
    {
        unsigned index = 0;
        auto *out = reinterpret_cast<float *>(&pose);
        const auto *tracks = state->Tracks();
        VisitChannels<Kind, QuatCount>(Mask(data), [&](unsigned offset, bool quaternion) {
            if (quaternion) {
                RetrieveQuaternion(out + offset, tracks + index);
                index += 3;
            } else
                out[offset] = tracks[index++].whole;
        });
    }
    void GetPose(Pose *out, uint32_t, Float time, Float, const nalChar::nalCharAnim *anim,
                 const typename T::PerSkelData *, const typename T::PerAnimData *data, const void *stream,
                 PerInstData *state, const T &)
    {
        float frameTime, blend;
        uint32_t next, current;
        anim->ComputeFrameValues(frameTime, next, current, blend, time);
        const auto *codecs = reinterpret_cast<const uint8_t *>(data) + 4;
        if (static_cast<int32_t>(current) != state->frame) {
            if (state->frame != -1 && static_cast<int32_t>(current) == state->frame + 1) {
                std::memcpy(&state->first, &state->second, sizeof(Pose));
            } else {
                uint32_t first;
                if (state->frame == -1 || static_cast<int32_t>(current) <= state->frame) {
                    first = 0;
                    state->decoder = CharChannelDecoder(stream, false);
                } else
                    first = state->frame + 2;
                for (uint32_t frame = first; frame <= current; ++frame)
                    AdvanceAnimDataOneFrame(state, data, anim, codecs, frame);
                RetrievePoseFromInst(state->first, state, data);
            }
            state->frame = current;
            if (!next)
                state->decoder = CharChannelDecoder(stream, false);
            AdvanceAnimDataOneFrame(state, data, anim, codecs, next);
            RetrievePoseFromInst(state->second, state, data);
        }
        auto *dst = reinterpret_cast<float *>(out);
        const auto *a = reinterpret_cast<const float *>(&state->first);
        const auto *b = reinterpret_cast<const float *>(&state->second);
        auto blendChannel = [&](unsigned offset, bool quaternion) {
            if (quaternion) {
                const auto result = math::Slerp(blend,
                                                vector4d(a[offset], a[offset + 1], a[offset + 2], a[offset + 3]),
                                                vector4d(b[offset], b[offset + 1], b[offset + 2], b[offset + 3]));
                for (unsigned i = 0; i < 4; ++i)
                    dst[offset + i] = result[i];
            } else if constexpr (Kind == PoseChannels::KnuckleCurl || Kind == PoseChannels::Curl ||
                                 Kind == PoseChannels::Reduced)
                dst[offset] = (1.0f - blend) * a[offset] + blend * b[offset];
            else
                dst[offset] = (b[offset] - a[offset]) * blend + a[offset];
        };
        if constexpr (Kind == PoseChannels::Reduced) {
            const uint32_t mask = Mask(data);
            for (unsigned i = 0; i < 2; ++i)
                if (mask & (1u << i))
                    blendChannel(4 * i, true);
            if (mask & 4)
                for (unsigned i = 8; i < 24; ++i)
                    blendChannel(i, false);
            for (unsigned i = 0; i < 20; ++i)
                if (mask & (4u << i))
                    blendChannel(24 + i, false);
        } else
            VisitChannels<Kind, QuatCount>(Mask(data), blendChannel);
    }
};
}  // namespace CharEntropyDecoder
