#include "debug_struct.h"

#include "variable.h"

#if !STANDALONE_SYSTEM
debug_struct_t & g_debug = var<debug_struct_t>(0x0095FF80);
#else
static debug_struct_t g_debug1 {};
debug_struct_t & g_debug = g_debug1;
#endif

