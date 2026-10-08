#include "nslbank.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <new>

VALIDATE_SIZE(nslWave, 0x28);
VALIDATE_OFFSET(nslWave, data, 0x1C);
VALIDATE_OFFSET(nslWave, sample_rate, 0x20);
VALIDATE_SIZE(nslWaveBank, 0x8C);
VALIDATE_OFFSET(nslWaveBank, wave_count, 0x40);
VALIDATE_OFFSET(nslWaveBank, waves, 0x44);
VALIDATE_OFFSET(nslWaveBank, file_id, 0x80);
VALIDATE_OFFSET(nslWaveBank, sample_data, 0x88);
VALIDATE_SIZE(nslBank, 0x34);

nslBank (&nsl_banks)[32] = var<nslBank[32]>(0x00946FD0);

#if STANDALONE_SYSTEM
namespace {
HANDLE s_bank_files[32] = {
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
    INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE,
};
uint32_t s_bank_priority;

enum class load_stage {
    idle,
    header,
    resident,
    sample_data,
};

struct bank_loader {
    int bank_index = -1;
    load_stage stage = load_stage::idle;
    OVERLAPPED request{};
    uint32_t expected_size;
    nslWaveBank header{};
    uint8_t *resident;
    uint8_t *sample_data;
};

bank_loader s_loader;

bool read_file_at(HANDLE file, uint32_t offset, void *destination, uint32_t size)
{
    OVERLAPPED request{};
    request.Offset = offset;
    DWORD ignored = 0;
    if (ReadFile(file, destination, size, &ignored, &request) == FALSE && GetLastError() != ERROR_IO_PENDING) {
        return false;
    }
    DWORD bytes_read = 0;
    return GetOverlappedResult(file, &request, &bytes_read, TRUE) != FALSE && bytes_read == size;
}

int wave_compare(const void *lhs, const void *rhs)
{
    const auto lhs_hash = static_cast<const nslWave *>(lhs)->name_hash;
    const auto rhs_hash = static_cast<const nslWave *>(rhs)->name_hash;
    return (lhs_hash > rhs_hash) - (lhs_hash < rhs_hash);
}

uint32_t bank_index(nslBankID bank_id)
{
    return (static_cast<uint32_t>(bank_id) >> 16) & 0x1Fu;
}

void release_bank(uint32_t index)
{
    auto &bank = nsl_banks[index];
    if (bank.wave_bank != nullptr) {
        delete[] reinterpret_cast<uint8_t *>(bank.wave_bank->sample_data);
        delete[] reinterpret_cast<uint8_t *>(bank.wave_bank);
    }
    if (s_bank_files[index] != INVALID_HANDLE_VALUE) {
        CloseHandle(s_bank_files[index]);
        s_bank_files[index] = INVALID_HANDLE_VALUE;
    }
    bank.wave_bank = nullptr;
    bank.state = 0;
}

void reset_loader(bool cancel_request)
{
    if (s_loader.bank_index >= 0 && cancel_request) {
        const auto file = s_bank_files[s_loader.bank_index];
        CancelIo(file);
        DWORD completed = 0;
        GetOverlappedResult(file, &s_loader.request, &completed, TRUE);
    }
    delete[] s_loader.sample_data;
    delete[] s_loader.resident;
    s_loader = {};
}

void fail_current_load()
{
    const auto index = static_cast<uint32_t>(s_loader.bank_index);
    const auto bank_id = nsl_banks[index].id;
    reset_loader(true);
    release_bank(index);
    nsl_banks[index].id = static_cast<nslBankID>(static_cast<uint32_t>(bank_id) + 0x200000u);
}

bool begin_loader_read(load_stage stage, uint32_t offset, void *destination, uint32_t size)
{
    s_loader.stage = stage;
    s_loader.request = {};
    s_loader.request.Offset = offset;
    s_loader.expected_size = size;
    DWORD ignored = 0;
    return ReadFile(s_bank_files[s_loader.bank_index], destination, size, &ignored, &s_loader.request) != FALSE ||
           GetLastError() == ERROR_IO_PENDING;
}

bool validate_header(const nslWaveBank &header)
{
    return std::memcmp(header.signature, "WAVEBK", 6) == 0 && header.major_version == '1' &&
           header.minor_version == '1' && header.resident_size >= sizeof(nslWaveBank);
}

bool finalize_loaded_bank()
{
    auto *wave_bank = reinterpret_cast<nslWaveBank *>(s_loader.resident);
    const auto waves_offset = reinterpret_cast<uint32_t &>(wave_bank->waves);
    if (waves_offset > wave_bank->resident_size ||
        wave_bank->wave_count > (wave_bank->resident_size - waves_offset) / sizeof(nslWave)) {
        return false;
    }

    wave_bank->waves = reinterpret_cast<nslWave *>(s_loader.resident + waves_offset);
    qsort(wave_bank->waves, wave_bank->wave_count, sizeof(nslWave), wave_compare);
    wave_bank->file_id = NFL_FILE_ID_INVALID;
    wave_bank->file_offset = 0;
    wave_bank->sample_data = reinterpret_cast<uint32_t>(s_loader.sample_data);
    for (uint32_t i = 0; i < wave_bank->wave_count; ++i) {
        auto &wave = wave_bank->waves[i];
        wave.file_id = NFL_FILE_ID_INVALID;
        if ((wave.flags & 2u) != 0) {
            wave.data += wave_bank->file_offset;
        } else {
            wave.data += wave_bank->sample_data;
        }
    }

    auto &bank = nsl_banks[s_loader.bank_index];
    bank.wave_bank = wave_bank;
    bank.state = 3;
    s_loader.resident = nullptr;
    s_loader.sample_data = nullptr;
    reset_loader(false);
    return true;
}

void start_next_load()
{
    if (s_loader.stage != load_stage::idle) {
        return;
    }
    auto selected = -1;
    auto priority = UINT32_MAX;
    for (int index = 0; index < 32; ++index) {
        if (nsl_banks[index].state == 1 && nsl_banks[index].priority < priority) {
            selected = index;
            priority = nsl_banks[index].priority;
        }
    }
    if (selected == -1) {
        return;
    }

    s_loader.bank_index = selected;
    nsl_banks[selected].state = 2;
    if (!begin_loader_read(load_stage::header, 0, &s_loader.header, sizeof(s_loader.header))) {
        fail_current_load();
    }
}

void update_bank_loader()
{
    if (s_loader.stage == load_stage::idle) {
        start_next_load();
        return;
    }

    DWORD bytes_read = 0;
    if (GetOverlappedResult(s_bank_files[s_loader.bank_index], &s_loader.request, &bytes_read, FALSE) == FALSE) {
        if (GetLastError() == ERROR_IO_INCOMPLETE) {
            return;
        }
        fail_current_load();
        return;
    }
    if (bytes_read != s_loader.expected_size) {
        fail_current_load();
        return;
    }

    if (s_loader.stage == load_stage::header) {
        if (!validate_header(s_loader.header)) {
            fail_current_load();
            return;
        }
        s_loader.resident = new (std::nothrow) uint8_t[s_loader.header.resident_size];
        if (s_loader.resident == nullptr ||
            !begin_loader_read(load_stage::resident, 0, s_loader.resident, s_loader.header.resident_size)) {
            fail_current_load();
        }
        return;
    }

    if (s_loader.stage == load_stage::resident) {
        const auto *wave_bank = reinterpret_cast<const nslWaveBank *>(s_loader.resident);
        if (!validate_header(*wave_bank)) {
            fail_current_load();
            return;
        }
        if (wave_bank->sample_data_size != 0) {
            s_loader.sample_data = new (std::nothrow) uint8_t[wave_bank->sample_data_size];
            if (s_loader.sample_data == nullptr || !begin_loader_read(load_stage::sample_data,
                                                                      wave_bank->resident_size,
                                                                      s_loader.sample_data,
                                                                      wave_bank->sample_data_size)) {
                fail_current_load();
            }
            return;
        }
    }

    if (!finalize_loaded_bank()) {
        fail_current_load();
    }
}
}  // namespace
#endif


nslBankID nslLoadBank(const char *path, int buffer)
{
#if STANDALONE_SYSTEM
    HANDLE file = CreateFileA(path,
                              GENERIC_READ,
                              FILE_SHARE_READ,
                              nullptr,
                              OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_OVERLAPPED,
                              nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return NSL_BANK_ID_INVALID;
    }

    uint32_t index = 0;
    while (index < 32 && nsl_banks[index].state != 0) {
        ++index;
    }
    if (index == 32) {
        CloseHandle(file);
        return NSL_BANK_ID_INVALID;
    }

    auto &bank = nsl_banks[index];
    const auto old_id = static_cast<uint32_t>(bank.id);
    const auto generation = ((((old_id >> 21) + 1u) << 21) & 0x7FFF0000u);
    bank.id = static_cast<nslBankID>(generation | (index << 16) | 0xFFFFu);
    bank.state = 1;
    bank.buffer = buffer;
    bank.file_id = NFL_FILE_ID_INVALID;
    bank.file_offset = 0;
    bank.priority = s_bank_priority++;
    bank.wave_bank = nullptr;
    s_bank_files[index] = file;
    return bank.id;
#else
    return static_cast<nslBankID>(CDECL_CALL(0x00798450, buffer, path, 0));
#endif
}

void nslFreeBank(nslBankID bank_id)
{
#if STANDALONE_SYSTEM
    const auto index = bank_index(bank_id);
    if (index < 32 && nsl_banks[index].id == bank_id) {
        if (s_loader.bank_index == static_cast<int>(index)) {
            reset_loader(true);
        }
        release_bank(index);
        nsl_banks[index].id = static_cast<nslBankID>(static_cast<uint32_t>(bank_id) + 0x200000u);
    }
#else
    CDECL_CALL(0x00798550, bank_id);
#endif
}

int nslGetBankState(nslBankID bank_id)
{
#if STANDALONE_SYSTEM
    const auto index = bank_index(bank_id);
    if (index >= 32 || nsl_banks[index].id != bank_id) {
        return -1;
    }
    return nsl_banks[index].state != 3;
#else
    return CDECL_CALL(0x007984D0, bank_id);
#endif
}

const nslWave *nslGetWave(nslWaveID wave_id)
{
    if (wave_id.value == UINT32_MAX) {
        return nullptr;
    }
    const auto index = (wave_id.value >> 16) & 0x1Fu;
    const auto &bank = nsl_banks[index];
    if (bank.state != 3 || static_cast<uint32_t>(bank.id) != (wave_id.value | 0xFFFFu) || bank.wave_bank == nullptr) {
        return nullptr;
    }
    const auto wave_index = wave_id.value & 0xFFFFu;
    return wave_index < bank.wave_bank->wave_count ? &bank.wave_bank->waves[wave_index] : nullptr;
}
const char *nslGetWaveGroupName(nslWaveID wave_id)
{
    const auto *wave = nslGetWave(wave_id);
    if (wave == nullptr) {
        return nullptr;
    }
#if STANDALONE_SYSTEM
    const auto *bank = nsl_banks[(wave_id.value >> 16) & 0x1Fu].wave_bank;
    if (bank->names == UINT32_MAX || wave->group == UINT32_MAX) {
        return nullptr;
    }
    return reinterpret_cast<const char *>(bank) + bank->names + wave->group;
#else
    return wave->group != 0 ? reinterpret_cast<const char *>(wave->group) + 264 : nullptr;
#endif
}


nslWaveID nslFindWave(uint32_t name_hash)
{
    if (name_hash == 0) {
        return {};
    }
    for (uint32_t bank_index = 0; bank_index < 32; ++bank_index) {
        const auto &bank = nsl_banks[bank_index];
        if (bank.state != 3 || bank.wave_bank == nullptr) {
            continue;
        }
        auto *wave = static_cast<const nslWave *>(bsearch(&name_hash,
                                                          bank.wave_bank->waves,
                                                          bank.wave_bank->wave_count,
                                                          sizeof(nslWave),
                                                          [](const void *key, const void *element) {
                                                              const auto hash = *static_cast<const uint32_t *>(key);
                                                              const auto wave_hash =
                                                                  static_cast<const nslWave *>(element)->name_hash;
                                                              return (hash > wave_hash) - (hash < wave_hash);
                                                          }));
        if (wave != nullptr) {
            const auto wave_index = static_cast<uint32_t>(wave - bank.wave_bank->waves);
            return nslWaveID{(static_cast<uint32_t>(bank.id) & 0xFFFF0000u) | wave_index};
        }
    }
    return {};
}

bool nslReadWaveData(nslWaveID wave_id, void *destination, uint32_t size)
{
#if STANDALONE_SYSTEM
    const auto *wave = nslGetWave(wave_id);
    if (wave == nullptr || size > wave->encoded_size) {
        return false;
    }
    if ((wave->flags & 2u) == 0) {
        std::memcpy(destination, reinterpret_cast<const void *>(wave->data), size);
        return true;
    }
    const auto index = (wave_id.value >> 16) & 0x1Fu;
    return s_bank_files[index] != INVALID_HANDLE_VALUE &&
           read_file_at(s_bank_files[index], wave->data, destination, size);
#else
    (void)wave_id;
    (void)destination;
    (void)size;
    return false;
#endif
}

unsigned int nslGetWaveChannelCount(nslWaveID wave_id)
{
    const auto *wave = nslGetWave(wave_id);
    if (wave == nullptr) {
        return 0;
    }
    auto mask = wave->channel_mask;
    if (mask == 0) {
        return 1;
    }
    mask = static_cast<uint8_t>((mask & 0x55u) + ((mask >> 1) & 0x55u));
    mask = static_cast<uint8_t>((mask & 0x33u) + ((mask >> 2) & 0x33u));
    return (mask & 0x0Fu) + (mask >> 4);
}

float nslGetWaveParam(nslWaveID wave_id, unsigned int parameter, float default_value)
{
    const auto *wave = nslGetWave(wave_id);
    if (wave == nullptr || parameter >= 0x2E) {
        return default_value;
    }
    const uint32_t *parameters;
#if STANDALONE_SYSTEM
    const auto *bank = nsl_banks[(wave_id.value >> 16) & 0x1Fu].wave_bank;
    if (bank->groups == UINT32_MAX || wave->parameters == UINT32_MAX) {
        return default_value;
    }
    parameters =
        reinterpret_cast<const uint32_t *>(reinterpret_cast<const uint8_t *>(bank) + bank->groups + wave->parameters);
#else
    parameters = reinterpret_cast<const uint32_t *>(wave->parameters);
    if (parameters == nullptr) {
        return default_value;
    }
#endif
    const uint64_t mask = uint64_t{parameters[0]} | (uint64_t{parameters[1]} << 32);
    const uint64_t bit = uint64_t{1} << parameter;
    if ((mask & bit) == 0) {
        return default_value;
    }
    auto preceding = mask & (bit - 1);
    unsigned int index = 0;
    while (preceding != 0) {
        preceding &= preceding - 1;
        ++index;
    }
    float value;
    std::memcpy(&value, parameters + 2 + index, sizeof(value));
    return value;
}

void nslUpdate()
{
#if STANDALONE_SYSTEM
    update_bank_loader();
#else
    CDECL_CALL(0x0079A770);
#endif
}

static auto &nsl_groups = var<float[8]>(0x00948470);
static auto &nsl_listener = var<float[8]>(0x00948598);

float *nsl_GetMaster()
{
    return nsl_groups;
}

float *nsl_GetListener()
{
    return nsl_listener;
}

void nsl_patch()
{
    REDIRECT(0x0054D4F2, nslUpdate);
    REDIRECT(0x0055234C, nslUpdate);
}
