#pragma once

namespace fs = std::filesystem;

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
// texture reads
typedef bool(__cdecl* nglLoadTextureTM2_t)(char* tex, uint8_t* a2);
nglLoadTextureTM2_t nglLoadTextureTM2;

bool hk_nglLoadTextureTM2(char* tex, uint8_t* a2)
{
    uint32_t hash = *(uint32_t*)((char*)(tex + 0x60));
    if (auto mod = getMod(hash, 1)) {
        a2 = mod->Data.data();
    }
    return nglLoadTextureTM2(tex, a2);
}


// resource reads
typedef unsigned __int8* (__cdecl* get_resource_t)(const resource_key*, int*, resource_pack_slot**);
get_resource_t get_resource_orig;

unsigned __int8* __cdecl hk_get_resource(const resource_key* resource_id, int* size, resource_pack_slot** data) {
    const uint32_t req_hash = resource_id->m_hash;
    const char* req_ext = resource_key_type_ext[PLATFORM][resource_id->m_type];

    uint8_t* ret = get_resource_orig(resource_id, size, data);
    if (!ret)
        return ret;

    printf(__FUNCTION__ ": searching for 0x%08X%s\n", req_hash, req_ext);
    auto mod = getModOfType(resource_id);
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

    printf(__FUNCTION__ ": searching for 0x%08X%s\n", req_hash, req_ext);
    if (Mod* mod = getModOfType(resource_id)) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());
        if (size) *size = static_cast<int>(mod->Data.size());
        return mod->Data.data();
    }
    return ret;
}
