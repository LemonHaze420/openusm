#include "pch.h"

#include <cstdint>
#include <MinHook.h>
#include <fstream>
#include <ostream>
#include <filesystem>

#include "Mod.h"

#if !defined(_DEBUG)
#define printf //
#endif

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
                resType = 1;
            auto hash = to_hash(path.stem().string().c_str());
            Mods[hash] = Mod{ path, resType, std::move(fileData) };
            printf(__FUNCTION__ " found name = %s\nhash = 0x%08X\n", path.stem().string().c_str(), hash);
        }
    }
}

typedef bool (__cdecl *nglLoadTextureTM2_t)(char* tex, uint8_t* a2);
nglLoadTextureTM2_t nglLoadTextureTM2;

bool hk_nglLoadTextureTM2(char* tex, uint8_t* a2)
{
    uint32_t hash = *(uint32_t*)((char*)(tex + 0x60));
    if (auto mod = getMod(hash, 1)) {
        a2 = mod->Data.data();
    }
    return nglLoadTextureTM2(tex, a2);
}

void init_hooks()
{
#   if PLATFORM==PLATFORM_PC
#       if defined(_DEBUG)
            AllocConsole();
            freopen("CONIN$", "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
#       endif

        MH_STATUS r = MH_Initialize();
        if (r == MH_OK) 
        {
            MH_CreateHook((void*)0x0077A870, reinterpret_cast<void*>(hk_nglLoadTextureTM2), reinterpret_cast<void**>(&nglLoadTextureTM2));
            r = MH_EnableHook((void*)0x0077A870);
            if (r == MH_OK) {
                enumerate_mods();
            }
        }
#   else
#   error "Unsupported platform"
#   endif
}

void destroy_hooks()
{
#   if PLATFORM==PLATFORM_PC
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
#   else
#   error "Unsupported platform"
#   endif
}

#if PLATFORM==PLATFORM_PC
BOOL APIENTRY DllMain( HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        {
            init_hooks();
            break;
        }
        case DLL_PROCESS_DETACH: {
            destroy_hooks();
            break;
        }
    }
    return TRUE;
}
#else
#error "Unsupported platform"
#endif