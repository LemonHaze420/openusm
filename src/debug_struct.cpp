#include "debug_struct.h"

#include "variable.h"
#include "variables.h"

#if !STANDALONE_SYSTEM
debug_struct_t & g_debug = var<debug_struct_t>(0x0095FF80);
#else
debug_struct_t & g_debug = []() -> auto & {
    static debug_struct_t g_debug1 {};
    return g_debug1;
}();
#endif

