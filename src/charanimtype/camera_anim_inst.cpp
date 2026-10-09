#include "camera_anim_inst.h"

#include "common.h"
#include "camera_skel_pose.h"
#include "variable.h"
#include "variables.h"
#include "charcompressor.h"
#include "nal_math.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>

#if STANDALONE_SYSTEM
namespace {
using Converter = CharEntropyQuantConverter;
struct camera_decode_state {
    CharEntropyDecoder::CharChannelDecoder channel;
    float first[12];
    float second[12];
    uint32_t count;
    int32_t frame;
    Converter::EncTrackData *tracks;
};
VALIDATE_SIZE(camera_decode_state, 0x74);
uint16_t camera_tracks(const nalCam::nalCamInstance *self)
{
    return *reinterpret_cast<const uint16_t *>(reinterpret_cast<const char *>(self->field_10) + 0x40);
}
const void *camera_stream(const nalCam::nalCamInstance *self)
{
    return *reinterpret_cast<const void *const *>(reinterpret_cast<const char *>(self->field_10) + 0x48);
}
void camera_advance_tracks(nalCam::nalCamInstance *self, uint32_t frame)
{
    auto *state = static_cast<camera_decode_state *>(self->decoder);
    const auto flags = camera_tracks(self);
    const float quant = *reinterpret_cast<const float *>(reinterpret_cast<const char *>(self->field_10) + 0x44);
    Converter::DecodeDequantTracks(
        state->tracks, self->codecs, state->channel, frame, 0, state->count, quant / 1024.0f, true);
    if (!frame)
        return;
    unsigned index = 0;
    if (flags & 1) {
        if (frame == 1)
            Converter::UnEntropyQuaternionTracksInitial(state->tracks, self->codecs, 0);
        else
            Converter::UnEntropyQuaternionTracks(state->tracks, self->codecs, 0);
        index = 3;
    }
    for (; index < state->count; ++index) {
        if (frame == 1)
            Converter::UnEntropyLinearTrackInitial(state->tracks, self->codecs, index);
        else
            Converter::UnEntropyLinearTrack(state->tracks, self->codecs, index);
    }
}
void camera_retrieve(nalCam::nalCamInstance *self, float *out)
{
    const auto *state = static_cast<const camera_decode_state *>(self->decoder);
    const auto flags = camera_tracks(self);
    unsigned index = 0;
    if (flags & 1) {
        for (unsigned i = 0; i < 3; ++i)
            out[i] = state->tracks[i].whole;
        out[3] = std::sqrt(std::fabs(1.0f - (out[0] * out[0] + out[1] * out[1] + out[2] * out[2])));
        for (unsigned i = 0; i < 3; ++i)
            out[4 + i] = state->tracks[3 + i].whole;
        index = 6;
    }
    if (flags & 2)
        out[8] = state->tracks[index++].whole;
    if (flags & 4)
        out[9] = state->tracks[index].whole;
}
void __fastcall camera_sample(nalCam::nalCamInstance *self, void *, Float time, Float, nalCam::nalCamPose *out,
                              const nalCam::nalCamPose *default_pose)
{
    *out = *default_pose;
    const auto *anim = self->field_10;
    const auto count = *reinterpret_cast<const uint16_t *>(reinterpret_cast<const char *>(anim) + 0x42);
    const bool looping = (anim->field_34 & 1) != 0;
    const float seconds = time.value * anim->field_38;
    const float rate = (looping ? count : count - 1) / anim->field_38;
    const double position = rate * seconds;
    unsigned next = static_cast<unsigned>(std::ceil(position));
    float blend = static_cast<float>(position - static_cast<unsigned>(position));
    if (next == static_cast<unsigned>(position)) {
        ++next;
        blend = 0.0f;
    }
    if (looping)
        next %= count;
    else {
        next = std::min(next, static_cast<unsigned>(count - 1));
        if (time.value > 1.0f)
            blend = 1.0f;
    }
    const unsigned current = next ? next - 1 : looping ? count - 1 : 0;
    auto *state = static_cast<camera_decode_state *>(self->decoder);
    if (current != static_cast<unsigned>(state->frame)) {
        if (state->frame != -1 && current == static_cast<unsigned>(state->frame + 1)) {
            std::memcpy(state->first, state->second, sizeof(state->first));
        } else {
            unsigned first = static_cast<unsigned>(state->frame + 2);
            if (state->frame == -1 || current < static_cast<unsigned>(state->frame)) {
                state->channel = CharEntropyDecoder::CharChannelDecoder(camera_stream(self), false);
                first = 0;
            }
            for (unsigned frame = first; frame <= current; ++frame)
                camera_advance_tracks(self, frame);
            camera_retrieve(self, state->first);
        }
        state->frame = current;
        if (!next)
            state->channel = CharEntropyDecoder::CharChannelDecoder(camera_stream(self), false);
        camera_advance_tracks(self, next);
        camera_retrieve(self, state->second);
    }
    auto *pose = reinterpret_cast<float *>(out);
    if (camera_tracks(self) & 1) {
        const auto rotation = math::Slerp(blend,
                                          {state->first[0], state->first[1], state->first[2], state->first[3]},
                                          {state->second[0], state->second[1], state->second[2], state->second[3]});
        for (unsigned i = 0; i < 4; ++i)
            pose[4 + i] = rotation[i];
        for (unsigned i = 0; i < 3; ++i)
            pose[8 + i] = state->first[4 + i] + (state->second[4 + i] - state->first[4 + i]) * blend;
    }
    if (camera_tracks(self) & 2)
        pose[1] = state->first[8] + (state->second[8] - state->first[8]) * blend;
    if (camera_tracks(self) & 8)
        pose[2] = state->first[9] + (state->second[9] - state->first[9]) * blend;
}
void *__fastcall camera_instance_destroy(nalCam::nalCamInstance *self, void *, unsigned flags)
{
    auto *state = static_cast<camera_decode_state *>(self->decoder);
    tlMemFree(state->tracks);
    tlMemFree(state);
    --self->field_10->InstanceCount;
    if (flags & 1)
        tlMemFree(self);
    return self;
}
}
#endif

namespace nalCam {
#if !STANDALONE_SYSTEM
int &nalCamAnim::vtbl_ptr = var<int>(0x0096AAF4);
#else
int &nalCamAnim::vtbl_ptr = []() -> auto & {
    static nalCamAnim g_anim{};
    return g_anim.m_vtbl;
}();
#endif
}  // namespace nalCam

nalCam::nalCamAnim::nalCamAnim()
{
    if constexpr (1) {
        static void *g_vtbl[]{nullptr,
                              func_address(&nalCamAnim::_Process),
                              func_address(&nalCamAnim::_Release),
                              func_address(&nalCamAnim::_CheckVersion),
                              func_address(&nalCamAnim::_VirtualCreateInstance)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x0089209C;
    }
}

void nalCam::nalCamAnim::_Process()
{
    auto *base = bit_cast<char *>(this);
    auto &data_end = *bit_cast<char **>(base + 0x48);
    auto &data = *bit_cast<char **>(base + 0x4C);
    data_end += bit_cast<std::intptr_t>(base + 0x50);
    data = base + 0x50;
}

void nalCam::nalCamAnim::_Release()
{
    auto *base = bit_cast<char *>(this);
    auto &data_end = *bit_cast<char **>(base + 0x48);
    auto &data = *bit_cast<char **>(base + 0x4C);
    data_end -= bit_cast<std::intptr_t>(base + 0x50);
    data = nullptr;
}

bool nalCam::nalCamAnim::_CheckVersion() const
{
    return Version == 0x10000;
}

nalCam::nalCamInstance *nalCam::nalCamAnim::CreateInstance(nalCam::nalCamSkeleton *a2)
{
    auto *result = new nalCamInstance(this, a2);
    return result;
}

nalCam::nalCamInstance *nalCam::nalCamAnim::_VirtualCreateInstance(nalBaseSkeleton *a1)
{
    return this->CreateInstance(bit_cast<nalCamSkeleton *>(a1));
}

nalCam::nalCamBaseInstance::nalCamBaseInstance(nalAnimClass<nalAnyPose> *a1, nalBaseSkeleton *a2)
    : nalAnimClass<nalAnyPose>::nalInstanceClass(a1, a2)
{
    this->m_vtbl = 0x00891AFC;
}

nalCam::nalCamInstance::nalCamInstance(nalCamAnim *a2, nalCam::nalCamSkeleton *a3) : nalCamBaseInstance(a2, a3)
{
#if STANDALONE_SYSTEM
    static void *table[]{reinterpret_cast<void *>(camera_instance_destroy), reinterpret_cast<void *>(camera_sample)};
    this->m_vtbl = reinterpret_cast<std::intptr_t>(table);
    const auto flags = camera_tracks(this);
    const unsigned count = ((flags & 1) ? 6u : 0u) + ((flags & 2) ? 1u : 0u) + ((flags & 4) ? 1u : 0u);
    auto *state = static_cast<camera_decode_state *>(tlMemAlloc(sizeof(camera_decode_state), 8, 0));
    new (state) camera_decode_state{
        CharEntropyDecoder::CharChannelDecoder(camera_stream(this), false),
        {},
        {},
        count,
        -1,
        static_cast<Converter::EncTrackData *>(tlMemAlloc(sizeof(Converter::EncTrackData) * count, 8, 0))};
    decoder = state;
    codecs = *reinterpret_cast<const uint8_t *const *>(reinterpret_cast<const char *>(a2) + 0x4C);
#else
    this->m_vtbl = 0x008920B0;
#endif
}
