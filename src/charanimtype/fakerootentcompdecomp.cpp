#include "fakerootentcompdecomp.h"

#include "fakerootposedesc.h"
#include "nativeentcompdecomp.h"
#include "func_wrapper.h"
#include "utility.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>

namespace {
using Pose = FakerootPoseDesc::StdPoseData;
using AnimData = FakerootPoseDesc::PerAnimData;
using State = FakerootEntCompDecomp<FakerootPoseDesc>::PerInstData;
using Converter = CharEntropyQuantConverter;

void BlendRoot(float *rotation, float *position, const float *aRotation, const float *aPosition, const float *bRotation,
               const float *bPosition, float blend)
{
    const auto q = math::Slerp(blend,
                               vector4d(aRotation[0], aRotation[1], aRotation[2], aRotation[3]),
                               vector4d(bRotation[0], bRotation[1], bRotation[2], bRotation[3]));
    for (unsigned i = 0; i < 4; ++i)
        rotation[i] = q[i];
    for (unsigned i = 0; i < 3; ++i)
        position[i] = (bPosition[i] - aPosition[i]) * blend + aPosition[i];
}
void BlendPose(Pose &out, const Pose &a, const Pose &b, float blend, uint32_t mask)
{
    if (mask & 1)
        BlendRoot(out.field_0, out.field_10, a.field_0, a.field_10, b.field_0, b.field_10, blend);
    if (mask & 2)
        out.field_1C = (1.0f - blend) * a.field_1C + blend * b.field_1C;
    out.field_20 = out.field_24 = 0;
}
struct RootTransform {
    float rotation[4];
    float position[3];
};


void Rotate(float *out, const float *p, const float *q)
{
    const float x = q[0], y = q[1], z = q[2], w = q[3];
    const float diagonal = 2 * w * w - 1;
    out[0] = (diagonal + 2 * x * x) * p[0] + (2 * x * y + 2 * w * z) * p[1] + (2 * x * z - 2 * w * y) * p[2];
    out[1] = (2 * x * y - 2 * w * z) * p[0] + (diagonal + 2 * y * y) * p[1] + (2 * y * z + 2 * w * x) * p[2];
    out[2] = (2 * x * z + 2 * w * y) * p[0] + (2 * y * z - 2 * w * x) * p[1] + (diagonal + 2 * z * z) * p[2];
}
RootTransform Inverse(const RootTransform &a)
{
    RootTransform out;
    for (unsigned i = 0; i < 3; ++i)
        out.rotation[i] = -a.rotation[i];
    out.rotation[3] = a.rotation[3];
    const float negative[3]{-a.position[0], -a.position[1], -a.position[2]};
    Rotate(out.position, negative, out.rotation);
    return out;
}
RootTransform Compose(const RootTransform &a, const RootTransform &b)
{
    RootTransform out;
    const float *q = a.rotation, *r = b.rotation;
    out.rotation[0] = r[3] * q[0] + q[1] * r[2] - q[2] * r[1] + r[0] * q[3];
    out.rotation[1] = q[1] * r[3] + q[3] * r[1] - r[2] * q[0] + r[0] * q[2];
    out.rotation[2] = q[0] * r[1] - q[1] * r[0] + r[3] * q[2] + r[2] * q[3];
    out.rotation[3] = r[3] * q[3] - (q[1] * r[1] + q[0] * r[0] + r[2] * q[2]);
    Rotate(out.position, a.position, b.rotation);
    for (unsigned i = 0; i < 3; ++i)
        out.position[i] += b.position[i];
    return out;
}
void Advance(State *state, const AnimData *data, const nalChar::nalCharAnim *anim, const uint8_t *codecs,
             uint32_t frame)
{
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
    if (data->field_1C & 1) {
        if (frame == 1)
            Converter::UnEntropyQuaternionTracksInitial(tracks, codecs, 0);
        else
            Converter::UnEntropyQuaternionTracks(tracks, codecs, 0);
        for (index = 3; index < 6; ++index) {
            if (frame == 1)
                Converter::UnEntropyLinearTrackInitial(tracks, codecs, index);
            else
                Converter::UnEntropyLinearTrack(tracks, codecs, index);
        }
    }
    if (data->field_1C & 2) {
        if (frame == 1)
            Converter::UnEntropyLinearTrackInitial(tracks, codecs, index);
        else
            Converter::UnEntropyLinearTrack(tracks, codecs, index);
    }
}
void Signals(Pose &pose, const AnimData *data, float time, float previousTime, const nalChar::nalCharAnim *anim)
{
    pose.field_20 = pose.field_24 = 0;
    auto it = data->GetStartIterator();
    if (!it.IsIteratorValid())
        return;
    const bool loop = (anim->field_34 & 1) != 0;
    const uint32_t frames = anim->GetTotalFrames();
    const double duration = double(loop ? frames : frames - 1) / anim->field_38;
    uint32_t end = static_cast<uint32_t>(std::ceil(duration * anim->field_38 * time));
    uint32_t start = static_cast<uint32_t>(std::ceil(duration * anim->field_38 * previousTime));
    if (loop) {
        end %= frames;
        start %= frames;
    } else {
        end = std::min(end, frames - 1);
        start = std::min(start, frames - 1);
    }
    if (start == end)
        return;

    bool found = false;
    do {
        const uint32_t frame = it.GetSignalFrame();
        if (start > end ? frame >= start : start <= frame && frame < end) {
            found = true;
            break;
        }
        if (start <= end && frame >= end)
            return;
        ++it;
    } while (!it.field_8);
    if (!found && !(start > end && it.GetSignalFrame() < end))
        return;
    pose.field_20 = it.m_pLoc->index;
    if (end < start && !it.field_8) {
        do {
            const uint32_t frame = it.GetSignalFrame();
            if (frame < start && frame >= end)
                break;
            ++pose.field_24;
            ++it;
        } while (!it.field_8);
    }
    do {
        if (it.GetSignalFrame() >= end)
            break;
        ++pose.field_24;
        ++it;
    } while (!it.field_8);
}
}  // namespace

template <>
void FakerootEntCompDecomp<FakerootPoseDesc>::RetrievePoseFromInst(Pose &pose, State *state, const AnimData *data)
{
    const auto *tracks = state->Tracks();
    unsigned index = 0;
    if (data->field_1C & 1) {
        CharEntropyDecoder::RetrieveQuaternion(pose.field_0, tracks);
        for (unsigned i = 0; i < 3; ++i)
            pose.field_10[i] = tracks[i + 3].whole;
        index = 6;
    }
    if (data->field_1C & 2)
        pose.field_1C = tracks[index].whole;
}

template <>
void FakerootEntCompDecomp<FakerootPoseDesc>::GetPose(Pose *out, uint32_t, Float time, Float previousTime,
                                                      const nalChar::nalCharAnim *anim,
                                                      const FakerootPoseDesc::PerSkelData *, const AnimData *data,
                                                      const void *stream, State *state, const FakerootPoseDesc &)
{
    const auto *codecs = reinterpret_cast<const uint8_t *>(data) + data->field_24;
    const uint32_t mask = data->field_1C;
    const bool root = (mask & 1) != 0;
    const bool loop = (anim->field_34 & 1) != 0;
    RootTransform cycle;
    if (root)
        std::memcpy(&cycle, data->field_0, sizeof(cycle));
    if ((mask & 7) == 4)
        Signals(*out, data, time, previousTime, anim);
    auto cache = [&](uint32_t current, uint32_t next) {
        if (static_cast<int32_t>(current) == state->frame)
            return;
        if (state->frame != -1 && static_cast<int32_t>(current) == state->frame + 1)
            std::memcpy(&state->first, &state->second, sizeof(Pose));
        else {
            uint32_t first;
            if (state->frame == -1 || static_cast<int32_t>(current) <= state->frame) {
                first = 0;
                state->decoder = CharEntropyDecoder::CharChannelDecoder(stream, false);
            } else
                first = state->frame + 2;
            for (uint32_t frame = first; frame <= current; ++frame)
                Advance(state, data, anim, codecs, frame);
            RetrievePoseFromInst(state->first, state, data);
        }
        state->frame = current;
        if (!next)
            state->decoder = CharEntropyDecoder::CharChannelDecoder(stream, false);
        Advance(state, data, anim, codecs, next);
        RetrievePoseFromInst(state->second, state, data);
    };
    float frameTime, blend;
    uint32_t next, current;
    anim->ComputeFrameValues(frameTime, next, current, blend, time);
    if (!std::equal_to<float>{}(previousTime, state->time)) {
        float previousFrameTime, previousBlend;
        uint32_t previousNext, previousCurrent;
        anim->ComputeFrameValues(previousFrameTime, previousNext, previousCurrent, previousBlend, previousTime);
        const bool endOfCycle = root && loop && previousCurrent > previousNext;
        if (endOfCycle) {
            previousNext = anim->GetTotalFrames() - 1;
            previousCurrent = previousNext - 1;
        }
        cache(previousCurrent, previousNext);
        if (root) {
            if (endOfCycle)
                BlendRoot(state->previousRotation,
                          state->previousPosition,
                          state->first.field_0,
                          state->first.field_10,
                          cycle.rotation,
                          cycle.position,
                          previousBlend);
            else
                BlendRoot(state->previousRotation,
                          state->previousPosition,
                          state->first.field_0,
                          state->first.field_10,
                          state->second.field_0,
                          state->second.field_10,
                          previousBlend);
        }
    }
    cache(current, next);
    BlendPose(*out, state->first, state->second, blend, mask);
    if (root) {
        if (loop && current > next)
            BlendRoot(out->field_0,
                      out->field_10,
                      state->first.field_0,
                      state->first.field_10,
                      cycle.rotation,
                      cycle.position,
                      blend);
        state->time = time;
        RootTransform absolute, previous;
        std::memcpy(&absolute, out, sizeof(absolute));
        std::memcpy(previous.rotation, state->previousRotation, sizeof(previous.rotation));
        std::memcpy(previous.position, state->previousPosition, sizeof(previous.position));
        const auto inverse = Inverse(previous);

        const auto delta = loop && static_cast<int64_t>(time) != static_cast<int64_t>(previousTime) &&
                                   !std::equal_to<float>{}(previousTime, 0.0f)
                               ? Compose(absolute, Compose(cycle, inverse))
                               : Compose(absolute, inverse);
        std::memcpy(out, &delta, sizeof(delta));
        std::memcpy(state->previousRotation, absolute.rotation, sizeof(absolute.rotation));
        std::memcpy(state->previousPosition, absolute.position, sizeof(absolute.position));
    }
    if (mask & 4)
        Signals(*out, data, time, previousTime, anim);
}

void FakerootEntCompDecomp_patch()
{
    auto func = &FakerootEntCompDecomp<FakerootPoseDesc>::GetPose;
    FUNC_ADDRESS(address, func);
    REDIRECT(0x005FE974, address);
}
