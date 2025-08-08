#include "pch.h"
#include "Mod.h"
#include "assets.h"
#include "scripting.h"

void destroy_hooks();

void init_hooks()
{
#   if PLATFORM==PLATFORM_PC
#       if defined(_DEBUG)
            AllocConsole();
            freopen("CONIN$", "r", stdin);
            freopen("CONOUT$", "w", stdout);
            freopen("CONOUT$", "w", stderr);
#       endif

        MH_STATUS ret = MH_Initialize();
        if (ret == MH_OK) 
        {
            struct Hook {
                void* pAddress;
                void* pHook;
                void** pOrig;
            } hooks[] = {
                {(void*)0x0077A870, reinterpret_cast<void*>(hk_nglLoadTextureTM2), reinterpret_cast<void**>(&nglLoadTextureTM2)},
                {(void*)0x00531B30, reinterpret_cast<void*>(hk_get_resource), reinterpret_cast<void**>(&get_resource_orig)},
                {(void*)0x0052AA70, reinterpret_cast<void*>(hk_get_resource_dir), reinterpret_cast<void**>(&get_resource_dir_orig)},
                //{(void*)0x0058EDE0, reinterpret_cast<void*>(hk_script_func_reg), reinterpret_cast<void**>(&script_func_reg_orig)},
                //{(void*)0x0058EE30, reinterpret_cast<void*>(hk_script_func), reinterpret_cast<void**>(&script_func_orig)},
                //{(void*)0x0064E740, reinterpret_cast<void*>(hk_exec), reinterpret_cast<void**>(&exec_orig)},
                //{(void*)0x005AF9F0, reinterpret_cast<void*>(hk_script_manager_run), reinterpret_cast<void**>(&script_manager_run_orig)},
            };

            for (auto& hook : hooks) {
                ret = MH_CreateHook(hook.pAddress, hook.pHook, hook.pOrig);
            }

            if (ret == MH_OK)
                ret = MH_EnableHook(MH_ALL_HOOKS);
        }

        if (ret == MH_OK) {
            enumerate_mods();
        }
        else
            destroy_hooks();
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