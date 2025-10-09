#pragma once

#include <unordered_map>
#include <filesystem>

#include "common.h"

struct Mod {
    std::filesystem::path Path;
    int Type;
    std::vector<uint8_t> Data;
};

static uint32_t current_pack = -1;

static std::unordered_map<uint64_t, std::vector<Mod>> Mods;

static inline uint64_t make_key(uint32_t nameHash, uint32_t dirHash) {
    return (uint64_t(dirHash) << 32) | nameHash;
}

[[maybe_unused]] static std::string transformToLower(const std::string& name)
{
    std::string res = name;
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::tolower(c); });
    return res;
}

[[maybe_unused]] static std::string transformToUpper(const std::string& name)
{
    std::string res = name;
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::toupper(c); });
    return res;
}

static std::vector<uint8_t> read_file(const std::filesystem::path& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
        return std::vector<uint8_t>();

    file.seekg(0, std::ios::end);
    std::streamsize sz = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(sz);
    buffer.resize(sz); buffer.reserve(sz);
    file.read(reinterpret_cast<char*>(buffer.data()), sz);
    return buffer;
}


inline constexpr unsigned char to_lower(unsigned char c) {
    constexpr auto delta = 'a' - 'A';

    if (c >= 'A' && c <= 'Z') {
        return (c + delta);
    }

    return c;
}

inline constexpr bool is_alpha(unsigned char c) {
    if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
        return true;
    }

    return false;
}

constexpr inline std::uint32_t to_hash(const char* str) {
    std::uint32_t res = 0;

    for (int c = *str; c != '\0'; ++str, c = *str) {
        int ch_lower = [](auto c) -> int {
            if (is_alpha(c)) {
                return to_lower(c);
            }

            return c;
            }(c);

        res = ch_lower + 33 * res;
    }

    return res;
}


Mod* getModOfType(const resource_key* resource_id, uint32_t dirHash)
{
    const uint32_t nameHash = resource_id->m_hash;
    const char* expected_ext = resource_key_type_ext[PLATFORM][resource_id->m_type];
    if (!expected_ext) return nullptr;

    auto it = Mods.find(make_key(nameHash, dirHash));
    if (it == Mods.end()) return nullptr;

    for (auto& mod : it->second) {
        if (transformToUpper(mod.Path.extension().string()) == transformToUpper(std::string(expected_ext)))
            return &mod;
    }
    return nullptr;
}