#include "nslsource.h"

#include "variables.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <memory>

namespace {
constexpr size_t source_count = 128;

constexpr WORD nsl_wave_format_adpcm = 0x0069;

struct source_slot {
    IDirectSoundBuffer *buffer;
    nslWaveID wave_id;
    uint16_t generation;
    bool allocated;
    float volume;
    float pitch;
    uint16_t pause_count;
    bool resume_playing;
};

source_slot s_sources[source_count]{};
uint16_t s_source_generation;

constexpr int ima_index_adjust[8] = {-1, -1, -1, -1, 2, 4, 6, 8};
constexpr int ima_step[89] = {
    7,    8,     9,     10,    11,    12,    13,    14,    16,    17,    19,    21,    23,    25,    28,
    31,   34,    37,    41,    45,    50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
    130,  143,   157,   173,   190,   209,   230,   253,   279,   307,   337,   371,   408,   449,   494,
    544,  598,   658,   724,   796,   876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
    2272, 2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,  7132,  7845,  8630,
    9493, 10442, 11487, 12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767,
};
constexpr float dsp_coefficients[5][2] = {
    {0.0f, 0.0f},
    {0.9375f, 0.0f},
    {1.796875f, -0.8125f},
    {1.53125f, -0.859375f},
    {1.90625f, -0.9375f},
};

source_slot *get_source(nslSourceID source_id)
{
    if (source_id.value == -1) {
        return nullptr;
    }
    const auto index = static_cast<uint16_t>(source_id.value);
    const auto generation = static_cast<uint16_t>(source_id.value >> 16);
    if (index >= source_count || generation == 0) {
        return nullptr;
    }
    auto &source = s_sources[index];
    return source.allocated && source.generation == generation ? &source : nullptr;
}

int16_t decode_ima_nibble(uint8_t nibble, int &previous_sample, int &step_index)
{
    const auto step = ima_step[step_index];
    auto difference = step >> 3;
    if ((nibble & 4u) != 0)
        difference += step;
    if ((nibble & 2u) != 0)
        difference += step >> 1;
    if ((nibble & 1u) != 0)
        difference += step >> 2;
    previous_sample += (nibble & 8u) != 0 ? -difference : difference;
    previous_sample = std::clamp(previous_sample, -32768, 32767);
    step_index = std::clamp(step_index + ima_index_adjust[nibble & 7u], 0, 88);
    return static_cast<int16_t>(previous_sample);
}

void decode_ima(const uint8_t *input, uint32_t size, uint32_t channels, int16_t *output)
{
    int previous_sample[2]{};
    int step_index[2]{};
    if (channels == 1) {
        for (uint32_t i = 0; i < size; ++i) {
            *output++ = decode_ima_nibble(input[i] & 0x0Fu, previous_sample[0], step_index[0]);
            *output++ = decode_ima_nibble(input[i] >> 4, previous_sample[0], step_index[0]);
        }
    } else {
        for (uint32_t i = 0; i < size; ++i) {
            *output++ = decode_ima_nibble(input[i] & 0x0Fu, previous_sample[0], step_index[0]);
            *output++ = decode_ima_nibble(input[i] >> 4, previous_sample[1], step_index[1]);
        }
    }
}

void decode_dsp_block(const uint8_t *input, int16_t *output, float &recent, float &older)
{
    const auto scale = input[0] & 0x0Fu;
    const auto predictor = static_cast<int8_t>(input[0]) >> 4;
    assert(predictor >= 0 && predictor < 5);
    for (int i = 0; i < 28; ++i) {
        const auto packed = input[2 + i / 2];
        auto residual = static_cast<int>(i & 1 ? packed & 0xF0u : (packed & 0x0Fu) << 4) << 8;
        residual = static_cast<int16_t>(residual) >> scale;
        const auto sample = static_cast<float>(residual) + recent * dsp_coefficients[predictor][0] +
                            older * dsp_coefficients[predictor][1];
        older = recent;
        recent = sample;
        output[i] = static_cast<int16_t>(std::lround(sample));
    }
}

void decode_dsp(const uint8_t *input, uint32_t size, uint32_t channels, int16_t *output)
{
    assert((size & 15u) == 0);
    if (channels == 1) {
        float recent = 0.0f;
        float older = 0.0f;
        for (uint32_t offset = 0; offset < size; offset += 16) {
            decode_dsp_block(input + offset, output, recent, older);
            output += 28;
        }
        return;
    }

    assert((size & 4095u) == 0);
    std::array<int16_t, 3584> left{};
    std::array<int16_t, 3584> right{};
    for (uint32_t chunk = 0; chunk < size; chunk += 4096) {
        float left_recent = 0.0f;
        float left_older = 0.0f;
        float right_recent = 0.0f;
        float right_older = 0.0f;
        for (uint32_t block = 0; block < 128; ++block) {
            decode_dsp_block(input + chunk + block * 16, left.data() + block * 28, left_recent, left_older);
            decode_dsp_block(input + chunk + 2048 + block * 16, right.data() + block * 28, right_recent, right_older);
        }
        for (size_t sample = 0; sample < left.size(); ++sample) {
            *output++ = left[sample];
            *output++ = right[sample];
        }
    }
}

struct adpcm_wave_format {
    WAVEFORMATEX format;
    WORD samples_per_block;
};

bool create_buffer(source_slot &source)
{
    const auto *wave = nslGetWave(source.wave_id);
    const auto channels = nslGetWaveChannelCount(source.wave_id);
    if (wave == nullptr || channels == 0 || channels > 2 || g_directSound == nullptr) {
        return false;
    }

    adpcm_wave_format wave_format{};
    auto &format = wave_format.format;
    format.nChannels = static_cast<WORD>(channels);
    format.nSamplesPerSec = wave->sample_rate;

    uint32_t buffer_size = 0;
    uint32_t input_size = 0;
    switch (wave->format) {
    case 1:
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.wBitsPerSample = 8;
        buffer_size = wave->sample_count * channels;
        input_size = buffer_size;
        break;
    case 2:
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.wBitsPerSample = 16;
        buffer_size = wave->sample_count * channels * 2;
        input_size = buffer_size;
        break;
    case 4:
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.wBitsPerSample = 16;
        buffer_size = wave->sample_count * 2;
        input_size = wave->encoded_size;
        break;
    case 5:
        format.wFormatTag = nsl_wave_format_adpcm;
        format.wBitsPerSample = 4;
        format.nBlockAlign = static_cast<WORD>(36 * channels);
        format.nAvgBytesPerSec = (wave->sample_rate >> 6) * format.nBlockAlign;
        format.cbSize = sizeof(WORD);
        wave_format.samples_per_block = 64;
        buffer_size = (wave->sample_count >> 6) * format.nBlockAlign;
        input_size = buffer_size;
        break;
    case 7:
        format.wFormatTag = WAVE_FORMAT_PCM;
        format.wBitsPerSample = 16;
        buffer_size = wave->encoded_size * 4;
        input_size = wave->encoded_size;
        break;
    default:
        assert(false && "Invalid non-streaming NSL wave format");
        return false;
    }

    if (format.wFormatTag == WAVE_FORMAT_PCM) {
        format.nBlockAlign = static_cast<WORD>(channels * format.wBitsPerSample / 8);
        format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
    }

    DSBUFFERDESC description{};
    description.dwSize = sizeof(description);
    description.dwFlags =
        DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    if (channels == 1)
        description.dwFlags |= DSBCAPS_CTRL3D;
    description.dwBufferBytes = buffer_size;
    description.lpwfxFormat = &format;
    if (FAILED(IDirectSound8_CreateSoundBuffer(g_directSound, &description, &source.buffer, nullptr))) {
        source.buffer = nullptr;
        return false;
    }

    std::unique_ptr<uint8_t[]> input{new (std::nothrow) uint8_t[input_size]};
    if (input == nullptr || !nslReadWaveData(source.wave_id, input.get(), input_size)) {
        IDirectSoundBuffer_Release(source.buffer);
        source.buffer = nullptr;
        return false;
    }

    void *output = nullptr;
    void *output_wrap = nullptr;
    DWORD output_size = 0;
    DWORD output_wrap_size = 0;
    if (FAILED(IDirectSoundBuffer_Lock(
            source.buffer, 0, buffer_size, &output, &output_size, &output_wrap, &output_wrap_size, 0)) ||
        output_size != buffer_size || output_wrap_size != 0) {
        IDirectSoundBuffer_Release(source.buffer);
        source.buffer = nullptr;
        return false;
    }

    if (wave->format == 4) {
        decode_dsp(input.get(), input_size, channels, static_cast<int16_t *>(output));
    } else if (wave->format == 7) {
        decode_ima(input.get(), input_size, channels, static_cast<int16_t *>(output));
    } else {
        std::memcpy(output, input.get(), buffer_size);
    }
    IDirectSoundBuffer_Unlock(source.buffer, output, output_size, output_wrap, output_wrap_size);
    if (channels == 1) {
        IDirectSound3DBuffer *spatial = nullptr;
        if (SUCCEEDED(IDirectSoundBuffer_QueryInterface(
                source.buffer, IID_IDirectSound3DBuffer, reinterpret_cast<void **>(&spatial)))) {
            IDirectSound3DBuffer_SetMode(spatial, DS3DMODE_DISABLE, DS3D_IMMEDIATE);
            IDirectSound3DBuffer_Release(spatial);
        }
    }
    return true;
}

LONG direct_sound_volume(float volume)
{
    if (volume <= 0.0f) {
        return DSBVOLUME_MIN;
    }
    return static_cast<LONG>(
        std::clamp(2000.0f * std::log10(volume), static_cast<float>(DSBVOLUME_MIN), static_cast<float>(DSBVOLUME_MAX)));
}
}  // namespace

nslSourceID nslCreateSource(nslWaveID wave_id)
{
    if (nslGetWave(wave_id) == nullptr) {
        return {};
    }
    size_t index = 0;
    while (index < source_count && s_sources[index].allocated) {
        ++index;
    }
    if (index == source_count) {
        return {};
    }
    if (++s_source_generation == 0) {
        ++s_source_generation;
    }
    auto &source = s_sources[index];
    source = {};
    source.wave_id = wave_id;
    source.generation = s_source_generation;
    source.allocated = true;
    source.volume = 1.0f;
    source.pitch = 1.0f;
    if (!create_buffer(source)) {
        source.allocated = false;
        return {};
    }
    return nslSourceID{static_cast<int32_t>((source.generation << 16) | index)};
}

bool nslPlaySource(nslSourceID source_id)
{
    auto *source = get_source(source_id);
    if (source == nullptr || source->buffer == nullptr) {
        return false;
    }
    IDirectSoundBuffer_SetCurrentPosition(source->buffer, 0);

    if (source->pause_count != 0) {
        source->resume_playing = true;
        return true;
    }
    return SUCCEEDED(IDirectSoundBuffer_Play(source->buffer, 0, 0, 0));
}

void nslStopSource(nslSourceID source_id)
{
    auto *source = get_source(source_id);
    if (source == nullptr) {
        return;
    }
    if (source->buffer != nullptr) {
        IDirectSoundBuffer_Stop(source->buffer);
        IDirectSoundBuffer_Release(source->buffer);
        source->buffer = nullptr;
    }
    source->allocated = false;
}

bool nslSourceIsPlaying(nslSourceID source_id)
{
    auto *source = get_source(source_id);
    if (source == nullptr || source->buffer == nullptr) {
        return false;
    }
    DWORD status = 0;
    return SUCCEEDED(IDirectSoundBuffer_GetStatus(source->buffer, &status)) && (status & DSBSTATUS_PLAYING) != 0;
}

void nslPauseSource(nslSourceID source_id)
{
    auto *source = get_source(source_id);
    if (source == nullptr || source->buffer == nullptr) {
        return;
    }
    if (source->pause_count++ == 0) {
        source->resume_playing = nslSourceIsPlaying(source_id);
        if (source->resume_playing) {
            IDirectSoundBuffer_Stop(source->buffer);
        }
    }
}

void nslUnpauseSource(nslSourceID source_id)
{
    auto *source = get_source(source_id);
    if (source == nullptr || source->buffer == nullptr || source->pause_count == 0) {
        return;
    }
    if (--source->pause_count == 0 && source->resume_playing) {
        IDirectSoundBuffer_Play(source->buffer, 0, 0, 0);
    }
}

bool nslSourceIsPaused(nslSourceID source_id)
{
    const auto *source = get_source(source_id);
    return source != nullptr && source->pause_count != 0;
}

void nslSetSourceVolume(nslSourceID source_id, float volume)
{
    auto *source = get_source(source_id);
    if (source != nullptr && source->buffer != nullptr) {
        source->volume = volume;
        IDirectSoundBuffer_SetVolume(source->buffer, direct_sound_volume(volume));
    }
}

void nslSetSourcePitch(nslSourceID source_id, float pitch)
{
    auto *source = get_source(source_id);
    const auto *wave = source != nullptr ? nslGetWave(source->wave_id) : nullptr;
    if (source != nullptr && source->buffer != nullptr && wave != nullptr) {
        source->pitch = pitch;
        const auto frequency = static_cast<DWORD>(std::clamp(static_cast<float>(wave->sample_rate) * pitch,
                                                             static_cast<float>(DSBFREQUENCY_MIN),
                                                             static_cast<float>(DSBFREQUENCY_MAX)));
        IDirectSoundBuffer_SetFrequency(source->buffer, frequency);
    }
}

void nslSetSourceSpatial(nslSourceID source_id, const float *position, const float *velocity, float min_distance,
                         float max_distance)
{
    auto *source = get_source(source_id);
    if (source == nullptr || source->buffer == nullptr)
        return;
    IDirectSound3DBuffer *spatial = nullptr;
    if (FAILED(IDirectSoundBuffer_QueryInterface(
            source->buffer, IID_IDirectSound3DBuffer, reinterpret_cast<void **>(&spatial))))
        return;
    IDirectSound3DBuffer_SetMode(spatial, DS3DMODE_NORMAL, DS3D_IMMEDIATE);
    IDirectSound3DBuffer_SetPosition(spatial, position[0], position[1], position[2], DS3D_IMMEDIATE);
    IDirectSound3DBuffer_SetVelocity(spatial, velocity[0], velocity[1], velocity[2], DS3D_IMMEDIATE);
    IDirectSound3DBuffer_SetMinDistance(spatial, min_distance, DS3D_IMMEDIATE);
    IDirectSound3DBuffer_SetMaxDistance(spatial, max_distance, DS3D_IMMEDIATE);
    IDirectSound3DBuffer_Release(spatial);
}

void nslReleaseSources()
{
    for (size_t index = 0; index < source_count; ++index) {
        if (s_sources[index].allocated) {
            nslStopSource(nslSourceID{static_cast<int32_t>((s_sources[index].generation << 16) | index)});
        }
    }
}
