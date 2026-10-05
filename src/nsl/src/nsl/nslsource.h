#pragma once

#include "nslbank.h"

#include <cstdint>

struct nslSourceID {
    int32_t value;

    constexpr nslSourceID(int32_t value = -1) : value(value) {}
};

extern nslSourceID nslCreateSource(nslWaveID wave_id);
extern bool nslPlaySource(nslSourceID source_id);
extern void nslStopSource(nslSourceID source_id);
extern bool nslSourceIsPlaying(nslSourceID source_id);
extern void nslSetSourceVolume(nslSourceID source_id, float volume);
extern void nslSetSourcePitch(nslSourceID source_id, float pitch);
extern void nslSetSourceSpatial(nslSourceID source_id, const float *position,
    const float *velocity, float min_distance, float max_distance);
extern void nslReleaseSources();
