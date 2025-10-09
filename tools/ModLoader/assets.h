#pragma once

#include <string_view>

namespace fs = std::filesystem;

#if 0
static void enumerate_mods() {
    fs::path modsDir = fs::current_path() / "mods";
    if (!fs::is_directory(modsDir))
        return;

    for (const auto& entry : fs::directory_iterator(modsDir)) {
        if (entry.is_regular_file()) {
            const fs::path& path = entry.path();
            std::vector<uint8_t> fileData = read_file(path);

            int resType = 0;
            std::string ext = transformToLower(path.extension().string());
            if (ext == ".dds" || ext == ".tga")
                resType = 1;    // @todo
            auto hash = to_hash(path.stem().string().c_str());
            Mods[hash] = Mod{ path, resType, std::move(fileData) };
            printf(__FUNCTION__ ": found name = %s\nhash = 0x%08X\n", path.stem().string().c_str(), hash);
        }
    }
}
#else
static void enumerate_mods() {
    namespace fs = std::filesystem;

    const fs::path modsDir = fs::current_path() / "mods";
    if (!fs::is_directory(modsDir))
        return;

    auto process_dir = [&](const fs::path& dirPath, uint32_t dirHash) {
        for (const auto& entry : fs::directory_iterator(dirPath)) {
            if (!entry.is_regular_file())
                continue;

            const fs::path& path = entry.path();
            std::vector<uint8_t> fileData = read_file(path);

            int resType = 0;
            std::string ext = transformToLower(path.extension().string());
            if (ext == ".dds" || ext == ".tga")
                resType = 1; // @todo

            const uint32_t nameHash = to_hash(path.stem().string().c_str());
            const uint64_t pathHash = make_key(nameHash, dirHash);

            Mods[pathHash].push_back(Mod{
                path,
                resType,
                std::move(fileData)
            });

            printf(__FUNCTION__ ": dir=%s (0x%08X)\n  name=%s (0x%08X)\n",
                dirPath.filename().string().c_str(), dirHash,
                path.stem().string().c_str(), nameHash);
        }
    };


    process_dir(modsDir, 0xFFFFFFFF);

    for (const auto& dir : fs::directory_iterator(modsDir)) {
        if (!dir.is_directory())
            continue;

        const std::string dirName = dir.path().filename().string();
        const uint32_t dirHash = to_hash(dirName.c_str());
        process_dir(dir.path(), dirHash);
    }
}


inline fs::path redirect_to_mods(std::string_view inPath)
{
    fs::path input(inPath);
    fs::path base = fs::current_path(); 
    fs::path mods = base / "mods";

    if (!input.is_absolute()) {
        return (fs::path("mods") / input).lexically_normal();
    }

    fs::path input_can, base_can;
    try {
        input_can = fs::weakly_canonical(input);
        base_can = fs::weakly_canonical(base);
    }
    catch (...) {
        return (mods / input.filename()).lexically_normal();
    }

    fs::path rel = fs::relative(input_can, base_can);

    bool is_under_base = true;
    for (const auto& part : rel) {
        if (part == "..") {
            is_under_base = false;
            break;
        }
    }

    if (is_under_base && !rel.empty())
        return (mods / rel).lexically_normal();
    return (mods / input_can.filename()).lexically_normal();
}
#endif

// texture reads
typedef bool(__cdecl* nglLoadTextureTM2_t)(char* tex, uint8_t* a2);
nglLoadTextureTM2_t nglLoadTextureTM2;

bool hk_nglLoadTextureTM2(char* tex, uint8_t* a2)
{
    uint32_t hash = *(uint32_t*)((char*)(tex + 0x60));
    resource_key key;
    key.m_hash = hash;
    key.m_type = 6;
    printf(__FUNCTION__ ": searching for 0x%08X\\0x%08X%s\n", current_pack, key.m_hash, resource_key_type_ext[PLATFORM][key.m_type]);

    if (auto mod = getModOfType(&key, current_pack)) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());
        a2 = mod->Data.data();
    }

    return nglLoadTextureTM2(tex, a2);
}


// resource reads
typedef unsigned __int8* (__cdecl* get_resource_t)(const resource_key*, int*, resource_pack_slot**);
get_resource_t get_resource_orig;

unsigned __int8* __cdecl hk_get_resource(const resource_key* resource_id, int* size, resource_pack_slot** slot) {
    const uint32_t req_hash = resource_id->m_hash;
    const char* req_ext = resource_key_type_ext[PLATFORM][resource_id->m_type];

    uint8_t* ret = get_resource_orig(resource_id, size, slot);
    if (!ret)
        return ret;

    printf(__FUNCTION__ ": searching for 0x%08X\\0x%08X%s\n", current_pack, req_hash, req_ext);
    auto mod = getModOfType(resource_id, current_pack);
    if (mod) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());

        if (size) *size = static_cast<int>(mod->Data.size());
        return mod->Data.data();
    }
    return ret;
}

typedef unsigned __int8* (__thiscall* get_resource_dir_t)(resource_directory*, const resource_key*, int*, resource_pack_slot**);
get_resource_dir_t get_resource_dir_orig;

unsigned __int8* __fastcall hk_get_resource_dir(resource_directory* self, void* edx, const resource_key* resource_id, int* size, resource_pack_slot** slot)
{
    uint32_t req_hash = resource_id->m_hash;
    uint32_t type = resource_id->m_type;
    const char* req_ext = resource_key_type_ext[PLATFORM][type];

    uint8_t* ret = get_resource_dir_orig(self, resource_id, size, slot);
    if (!ret)
        return ret;

    printf(__FUNCTION__ ": searching for 0x%08X\\0x%08X%s\n", current_pack, req_hash, req_ext);
    if (Mod* mod = getModOfType(resource_id, current_pack)) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());

        if (size) *size = static_cast<int>(mod->Data.size());
        return mod->Data.data();
    }
    return ret;
}

typedef void (__cdecl* set_active_resource_context_t)(resource_pack_slot* pack_slot);
set_active_resource_context_t set_active_resource_context_orig;

void hk_set_active_resource_context(resource_pack_slot* pack_slot)
{
    set_active_resource_context_orig(pack_slot);
    if (pack_slot)
        current_pack = pack_slot->field_4.m_hash;
}


typedef int (__cdecl* nflopenfile_t)(int, const char*);
nflopenfile_t nflopenfile_orig;

int hk_nflopenfile(int type, const char* str)
{
    auto modPath = redirect_to_mods(str);
    printf("searching for %s\n", modPath.string().c_str());
    if (fs::exists(modPath)) {
        printf("found %s\n", modPath.string().c_str());
        return nflopenfile_orig(type, modPath.string().c_str());
    }
    return nflopenfile_orig(type, str);
}