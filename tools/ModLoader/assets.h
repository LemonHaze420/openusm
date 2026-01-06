#pragma once

#include <string_view>


namespace fs = std::filesystem;

static void enumerate_mods() {
    namespace fs = std::filesystem;

    const fs::path modsDir = fs::current_path() / MOD_DIR;
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
            else if (ext == ".pcmesh")  // @todo: other exts
                resType = TLRESOURCE_TYPE_MESH_FILE;

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


    process_dir(modsDir, rootDir);

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
    fs::path mods = base / MOD_DIR;

    if (!input.is_absolute()) {
        return (fs::path(MOD_DIR) / input).lexically_normal();
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



struct generic_mash_header {
    int32_t total_size;
    int32_t flags;
    int32_t class_id;
    int32_t pad;
};

typedef void(__cdecl* set_active_resource_context_t)(resource_pack_slot* pack_slot);
set_active_resource_context_t set_active_resource_context_orig;

void hk_set_active_resource_context(resource_pack_slot* pack_slot)
{
    set_active_resource_context_orig(pack_slot);
    if (pack_slot)
        current_pack = pack_slot->field_4.m_hash;
}


typedef int(__cdecl* nflopenfile_t)(int, const char*);
nflopenfile_t nflopenfile_orig;

int hk_nflopenfile(int type, const char* str)
{
    auto modPath = redirect_to_mods(str);
    printf(__FUNCTION__": searching for %s\n", modPath.string().c_str());
    if (fs::exists(modPath)) {
        printf(__FUNCTION__": found %s\n", modPath.string().c_str());
        return nflopenfile_orig(type, modPath.string().c_str());
    }
    return nflopenfile_orig(type, str);
}


// texture reads
typedef bool(__cdecl* nglLoadTextureTM2_t)(char* tex, uint8_t* a2);
nglLoadTextureTM2_t nglLoadTextureTM2;

bool hk_nglLoadTextureTM2(char* tex, uint8_t* a2)
{
    uint32_t hash = *(uint32_t*)((char*)(tex + 0x60));
    resource_key key;
    key.m_hash = hash;
    key.m_type = 6;
    //printf(__FUNCTION__ ": searching for %s\\0x%08X%s\n", current_pack != rootDir ? lookup_string(current_pack) : ".", key.m_hash, resource_key_type_ext[PLATFORM][key.m_type]);

    if (auto mod = getModOfResType(&key, current_pack)) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());
        a2 = reinterpret_cast<uint8_t*>(mod->Data.data());
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
    if (!ret || resource_id->m_type == 3)
        return ret;


    auto packhash = current_pack;
    printf(__FUNCTION__ ": searching for %s\\%s%s\n", lookup_string(packhash), lookup_string(req_hash), req_ext);
    auto mod = getModOfResType(resource_id, packhash);
    if (mod) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());
        if (mod) {
            if (size) *size = (int)mod->Data.size();
            ret = reinterpret_cast<uint8_t*>(mod->Data.data());
        }
        printf("ret = 0x%08X\n", ret);
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

    uint32_t dirHash = current_pack;
    if (self->pack_slot)
        dirHash = self->pack_slot->field_4.m_hash;

    printf(__FUNCTION__ ": searching for %s\\%s%s\n", lookup_string(dirHash), lookup_string(req_hash), req_ext);
    if (Mod* mod = getModOfResType(resource_id, dirHash)) {
        printf(__FUNCTION__ ": found %s\n", mod->Path.string().c_str());
        if (mod) {
            if (size) *size = (int)mod->Data.size();
            ret = reinterpret_cast<uint8_t*>(mod->Data.data());
        }
        printf("ret = 0x%08X\n", ret);
        return ret;
    }
    return ret;
}

struct tlFixedString {
    uint32_t hash;
    char name[28];
};

typedef char* (__thiscall * get_tlresource_t)(resource_directory* self, tlFixedString* out_loc_, tlresource_type tlres_type);
get_tlresource_t get_tlresource_orig;


char* __fastcall hk_get_tlresource(resource_directory* self, void* edx, tlFixedString* out_loc_, tlresource_type tlres_type)
{
    char* ret = get_tlresource_orig(self, out_loc_, tlres_type);

    if (ret) {

        uint32_t dirHash = rootDir;
        if (self->pack_slot) {
            dirHash = self->pack_slot->field_4.m_hash;
        }
        const char* ext = nullptr;
        auto iter = tlres_to_res_key.find(tlres_type);
        if (iter != tlres_to_res_key.end()) {
            ext = resource_key_type_ext[PLATFORM][iter->second];

            auto hash = out_loc_->hash;
            char* s = lookup_string(hash);
            printf(__FUNCTION__ ": searching for %s\\%s%s\n", lookup_string(dirHash), s ? s : std::format("0x{:08X}", hash).c_str(), ext);

            if (auto mod = findModOfTlResType(make_key(out_loc_->hash, self->pack_slot->field_4.m_hash), tlres_type)) {
                ret = reinterpret_cast<char*>(mod->Data.data());
            }
        }
    }


    return ret;
}


typedef tlresource_location* (__thiscall * get_tlresource_loc_t)(resource_directory* self, unsigned __int16 idx, tlresource_type a3);
get_tlresource_loc_t get_tlresource_loc_orig;

tlresource_location* __fastcall hk_get_tlresource_loc(resource_directory* self, void* edx, unsigned __int16 idx, tlresource_type a3)
{
    tlresource_location* ret = get_tlresource_loc_orig(self, idx, a3);

    auto iter = tlres_to_res_key.find(a3);
    if (iter != tlres_to_res_key.end()) {
        const char* ext = resource_key_type_ext[PLATFORM][iter->second];
        uint32_t packhash = self->pack_slot->field_4.m_hash;
        uint32_t hash = make_key(ret->name.source_hash_code, packhash);
        char* s = lookup_string(ret->name.source_hash_code);
        printf(__FUNCTION__ ": searching for %s\\%s%s\n", lookup_string(packhash), s ? s : std::format("0x{:08X}", ret->name.source_hash_code).c_str(), ext);

        if (auto mod = findModOfTlResType(hash, a3)) {
            ret->mesh_file = reinterpret_cast<char*>(mod->Data.data());
        }
    }
    return ret;
}
