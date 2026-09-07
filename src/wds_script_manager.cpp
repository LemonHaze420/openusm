#include "wds_script_manager.h"
#include "script.h"
#include "script_manager.h"
#include "script_object.h"

#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

int wds_script_manager::hook_up_global_script_object()
{
    TRACE("wds_script_manager::hook_up_global_script_object");

    auto *global_object = script_manager::find_global_object();
    field_0 = reinterpret_cast<int>(global_object);
    auto *global_instance = global_object != nullptr ? global_object->get_global_instance() : nullptr;
    field_4 = reinterpret_cast<int>(global_instance);
#if STANDALONE_SYSTEM
    script::gso = global_object;
    script::gsoi = global_instance;
#else
    script::gso() = global_object;
    script::gsoi() = global_instance;
#endif
    return field_4;
}

void wds_script_manager_patch()
{
    FUNC_ADDRESS(address, &wds_script_manager::hook_up_global_script_object);
    REDIRECT(0x0055CC79, address);
}
