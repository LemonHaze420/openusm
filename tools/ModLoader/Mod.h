#pragma once

#include <unordered_map>
#include <filesystem>

#include "common.h"

struct Mod {
    std::filesystem::path Path;
    int Type;
    std::vector<uint8_t> Data;
    uint32_t Dir;
};

static uint32_t current_pack = -1;

static std::unordered_map<uint64_t, Mod> Mods;

static inline uint64_t make_key(uint32_t nameHash, uint32_t dirHash) {
    return (uint64_t(dirHash) << 32) | nameHash;
}

[[maybe_unused]] static bool hasMod(uint32_t hash) {
    return Mods.find(hash) != Mods.end();
}

[[maybe_unused]] static Mod* getMod(uint32_t hash, int type = -1) {
    if (type == -1) {
        auto it = Mods.find(hash);
        if (it != Mods.end())
            return &it->second;
    }
    else {
        for (auto& [ihash, mod] : Mods)
            if (hash == ihash && mod.Type == type)
                return &mod;
    }
    return nullptr;
}
[[maybe_unused]] static uint8_t* getModDataByHash(uint32_t hash) {
    if (hasMod(hash))
        if (auto mod = getMod(hash))
            return &mod->Data.data()[0];
    return nullptr;
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

// this is O(n) (don't use this unless necessary!)
[[maybe_unused]] static Mod* getModByFilemame(const std::string& name) {
    std::string search = transformToLower(name);
    for (auto& [hash, mod] : Mods) {
        std::string filename = transformToLower(mod.Path.filename().string());
        if (filename == search)
            return &mod;
    }
    return nullptr;
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

    return transformToUpper(it->second.Path.extension().string())
        == transformToUpper(std::string(expected_ext))
        ? &it->second : nullptr;
}