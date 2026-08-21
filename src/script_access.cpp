#include "script_access.h"

#include "script_object.h"
#include "trace.h"
#include "utility.h"

script_instance *script::create_instance(string_hash a1, script_object *a2)
{
    TRACE("script::create_instance");

    script::thread() = nullptr;
    script_instance *v11 = nullptr;
    if (a2 != nullptr) {
        auto **v6 = &script::thread();
        char *v5 = nullptr;
        v11 = a2->add_instance(a1, v5, v6);
        assert(thread() != nullptr);
    }

    return v11;
}

void script_access_patch()
{
    SET_JUMP(0x0064E9F0, &script::create_instance);
}
