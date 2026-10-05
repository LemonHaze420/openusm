#pragma once

#include "nfl_system.h"
#include "variable.h"

#include <cstddef>
#include <cstdint>

enum nslBankID : int32_t {
    NSL_BANK_ID_INVALID = -1,
};

struct nslWaveID {
    uint32_t value;

    constexpr nslWaveID(uint32_t value = UINT32_MAX) : value(value) {}
};

struct nslWave {
    uint32_t name_hash;
    uint8_t format;
    uint8_t flags;
    uint8_t channel_mask;
    uint8_t field_7;
    uint32_t encoded_size;
    uint32_t sample_count;
    uint32_t group;
    uint32_t parameters;
    uint32_t name;
    uint32_t data;
    uint16_t sample_rate;
    uint16_t field_22;
    nflFileID file_id;
};

struct nslWaveBank {
    char signature[6];
    char major_version;
    char minor_version;
    uint32_t field_8;
    uint32_t field_C;
    uint32_t resident_size;
    uint32_t sample_data_size;
    uint32_t sample_data_offset;
    uint32_t field_1C;
    char name[32];
    uint32_t wave_count;
    nslWave *waves;
    uint32_t field_48[2];
    uint32_t group_count;
    uint32_t groups;
    uint32_t parameter_count;
    uint32_t parameters;
    uint32_t name_count;
    uint32_t names;
    uint32_t field_68[6];
    nflFileID file_id;
    uint32_t file_offset;
    uint32_t sample_data;
};

struct nslBank {
    nslBankID id;
    int state;
    int buffer;
    nflFileID file_id;
    uint32_t file_offset;
    uint32_t priority;
    nslWaveBank *wave_bank;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    int field_30;
};

extern nslBank (&nsl_banks)[32];


extern nslBankID nslLoadBank(const char *path, int buffer);
extern void nslFreeBank(nslBankID bank_id);
extern int nslGetBankState(nslBankID bank_id);
extern nslWaveID nslFindWave(uint32_t name_hash);
extern const nslWave *nslGetWave(nslWaveID wave_id);
extern const char *nslGetWaveGroupName(nslWaveID wave_id);
extern bool nslReadWaveData(nslWaveID wave_id, void *destination, uint32_t size);
extern unsigned int nslGetWaveChannelCount(nslWaveID wave_id);
extern float nslGetWaveParam(nslWaveID wave_id, unsigned int parameter, float default_value);

extern void nslUpdate();
extern float *nsl_GetMaster();
extern float *nsl_GetListener();
extern void nsl_patch();
