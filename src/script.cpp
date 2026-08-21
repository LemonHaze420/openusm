#include "script.h"

#include "func_wrapper.h"
#include "log.h"
#include "script_object.h"
#include "utility.h"

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
}  // namespace script

void script_patch()
{
    SET_JUMP(0x0064E4F0, script::find_function);
}
