#include "chuck_callbacks.h"

#include "func_wrapper.h"
#include "game.h"
#include "event_manager.h"
#include "event_recipient_entry.h"
#include "event_type.h"
#include "vm_thread.h"
#include "osassert.h"
#include "resource_manager.h"
#include "script_controller.h"
#include "trace.h"
#include "vm_executable.h"
#include "utility.h"

extern void vm_executable_resolve_signal_callback(const char *name, unsigned int *hash);

void script_manager_callback(script_manager_callback_reason a1, script_executable *a2, const char *buffer)
{
    TRACE("script_manager_callback", std::to_string(int(a1)).c_str());

    switch (a1) {
    case 0:
    case 2:
    case 3:
    case 6:
    case 7:
        return;
    case 1:
        assert(g_game_ptr != nullptr);
        event_manager::clear_script_callbacks(script_pad[0].my_handle, a2);
        event_manager::clear_script_callbacks(script_pad[1].my_handle, a2);
        break;
    case 4:
        assert(buffer != nullptr);
        sp_log(buffer);
        assert(0);
        break;
    case 5:
        assert(buffer != nullptr);
        warning(buffer);
        break;
    case 8:
    case 10:
        if (buffer != nullptr) {
            resource_manager::push_resource_context(bit_cast<resource_pack_slot *>(buffer));
        }
        break;
    case 9:
    case 11:
        resource_manager::pop_resource_context();
        break;
    default:
        assert(0 && "unknown reason for script manager to call me:(");
        return;
    }
}

#if STANDALONE_SYSTEM
namespace {
void add_signal_callback(vm_thread *, string_hash signal, vhandle_type<signaller> owner, script_instance *instance,
                         vm_executable *function, char *parameters, bool one_shot)
{
    auto *type = event_manager::register_event_type(signal, false);
    auto *recipient = type->create_recipient_entry(owner.field_0);
    recipient->add_callback(instance, function, parameters, one_shot);
}

void raise_signal(vm_thread *, string_hash signal, vhandle_type<signaller> owner)
{
    event_manager::raise_event(signal, owner.field_0);
}

void raise_all_signals(vm_thread *, string_hash signal)
{
    if (auto *type = event_manager::get_event_type(signal)) {
        const auto count = type->field_8.size();
        for (unsigned index = 0; index < count; ++index)
            type->raise_event(type->field_8[index]->get_my_vhandle(), nullptr);
    }
}
}
#endif

void register_chuck_callbacks()
{
    TRACE("register_chuck_callbacks");

    script_manager::register_callback(script_manager_callback);
    vm_executable::resolve_signal_callback = vm_executable_resolve_signal_callback;
#if STANDALONE_SYSTEM
    vm_thread::add_signal_callback_callback = add_signal_callback;
    vm_thread::raise_signal_callback = raise_signal;
    vm_thread::raise_all_signal_callback = raise_all_signals;
#endif
}

void chuck_callbacks_patch()
{
    SET_JUMP(0x00660760, script_manager_callback);
}
