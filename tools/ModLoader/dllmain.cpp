#include "pch.h"
#include "Mod.h"
#include "assets.h"
#include "scripting.h"

void destroy_hooks();

void init_hooks()
{
#   if WIP_SCRIPTING
        init_scripting();
        REDIRECT(0x005AD77D, hk_script_ctor);
#   endif

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
#           define HOOK(a,b,c)     (void*)a, reinterpret_cast<void*>(b), reinterpret_cast<void**>(&c)
            struct Hook {
                void* pAddress;
                void* pHook;
                void** pOrig;
            } hooks[] = {
                { HOOK(0x0077A870, hk_nglLoadTextureTM2, nglLoadTextureTM2) },
                { HOOK(0x00531B30, hk_get_resource, get_resource_orig) },
                { HOOK(0x0052AA70, hk_get_resource_dir, get_resource_dir_orig) },
                { HOOK(0x0051EC80, hk_set_active_resource_context, set_active_resource_context_orig) },
                { HOOK(0x0079E490, hk_nflopenfile, nflopenfile_orig ) },
#               if WIP_SCRIPTING
                    { HOOK(0x0058EDE0, hk_script_func_reg, script_func_reg_orig) },
                    { HOOK(0x0058EE30, hk_script_func, script_func_orig) },
                    { HOOK(0x0064E740, hk_exec, exec_orig) },
                    { HOOK(0x005AF9F0, hk_script_manager_run, script_manager_run_orig) },
#               endif  
            };
#           undef HOOK

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
#       error "Unsupported platform"
#   endif
}

void destroy_hooks()
{
#      if PLATFORM==PLATFORM_PC
            MH_DisableHook(MH_ALL_HOOKS);
            MH_Uninitialize();

#       if WIP_SCRIPTING
            destroy_scripting();
#       endif

#   else
#       error "Unsupported platform"
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