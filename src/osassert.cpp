#include "osassert.h"

#include "debugutil.h"
#include "debug_struct.h"
#include "os_developer_options.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>

void warning(const char *arg0, ...)
{
    va_list va;
    va_start(va, arg0);
    if ( (g_debug.field_0 & debug_struct_t::OUTPUT_WARNING) != 0 )
    {
        va_list v2;
        va_copy(v2, va);

        char a1[2048];
        vsnprintf(a1, 2048, arg0, va);
        if ( os_developer_options::instance->get_flag(mString {"ASSERT_ON_WARNING"}) ) {
            assert(a1);
        } else {
            debug_print_va(a1);
        }
    }
}

void error(const char *arg0, ...)
{
    va_list va;
    va_start(va, arg0);

    va_list v2;
    va_copy(v2, va);

    char a1[2048];
    vsnprintf(a1, 2048, arg0, va);
    g_debug.field_1 |= 1u;
    assert(a1);
}
