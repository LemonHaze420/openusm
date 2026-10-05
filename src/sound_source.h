#pragma once

#include "nslbank.h"
#include "sound_alias_database.h"


struct sound_source {
    nslWaveID wave_id;
    sound_alias *alias = nullptr;

    bool is_valid() const
    {
        const auto *wave = nslGetWave(wave_id);
        return wave != nullptr && wave->name_hash != 0;
    }

    float get_min_distance() const
    {
        return alias != nullptr ? alias->min_distance : nslGetWaveParam(wave_id, 0x19, 0.0f);
    }

    float get_max_distance() const
    {
        return alias != nullptr ? alias->max_distance : nslGetWaveParam(wave_id, 0x1A, 0.0f);
    }
};

static_assert(sizeof(sound_source) == 8);
