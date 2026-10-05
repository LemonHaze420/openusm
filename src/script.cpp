#include "script.h"

#include "func_wrapper.h"
#include "log.h"
#include "script_object.h"
#include "utility.h"
#include "script_access.h"
#include "vm_thread.h"
#include "vm_stack.h"
#include "entity_base.h"
#include "actor.h"
#include "advanced_entity_ptrs.h"
#include "vector3d.h"

#include <cassert>

namespace script {
int find_function(string_hash a1, const script_object *a2, [[maybe_unused]] bool a3)
{
    {
        auto *str = a1.to_string();

        sp_log("%s", str);
    }

    if (a2 == nullptr) {
        assert(0 && "Script has not been initted yet!");
    }

    int result = a2->find_func(a1);

    if (result < 0) {
        result = -1;
    }

    return result;
}

vm_thread *new_thread(int function, script_instance *instance)
{
    thread() = nullptr;
    if (instance == nullptr || function < 0)
        return nullptr;
    thread() = instance->add_thread(instance->parent->get_func(function));
    if (instance != get_gsoi())
        thread()->get_data_stack().push(reinterpret_cast<const char *>(&instance), sizeof(instance));
    return thread();
}

bool push_arg(entity_base *entity)
{
    if (thread() == nullptr)
        return false;
    const entity_base_vhandle handle = entity != nullptr ? entity->my_handle : INVALID_HANDLE;
    thread()->get_data_stack().push(reinterpret_cast<const char *>(&handle), sizeof(handle));
    return true;
}

bool push_arg(const vector3d &vector)
{
    if (thread() == nullptr)
        return false;
    thread()->get_data_stack().push(reinterpret_cast<const char *>(&vector), sizeof(vector));
    return true;
}

bool exec_thread(bool immediate)
{
    auto *current = thread();
    if (current == nullptr)
        return false;
    thread() = nullptr;
    if (immediate)
        current->inst->run_single_thread(current, true);
    return true;
}
}  // namespace script

vm_thread *spawn_thread_for_func(string_hash function, script_instance *instance)
{
    const int index = script::find_function(function, instance->parent, false);
    return index >= 0 ? script::new_thread(index, instance) : nullptr;
}

vm_thread *find_func_and_spawn_new_thread(actor *owner, string_hash function)
{
    if (owner != nullptr && owner->adv_ptrs != nullptr) {
        if (auto *instance = owner->adv_ptrs->my_script) {
            const int index = script::find_function(function, instance->parent, true);
            if (index >= 0)
                return script::new_thread(index, instance);
        }
    }
    if (auto *instance = script::get_gsoi()) {
        const int index = script::find_function(function, instance->parent, false);
        if (index >= 0)
            return script::new_thread(index, instance);
    }
    return nullptr;
}

void script_patch()
{
    SET_JUMP(0x0064E4F0, script::find_function);
}
