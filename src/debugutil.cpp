#include "debugutil.h"

#include "log.h"

#include <cassert>
#include <cstdio>
#include <stdarg.h>
#include <string.h>

void debug_print_va(const char *fmtp, ...)
{
    va_list va;
    va_start(va, fmtp);

    assert(strlen(fmtp) < 2047 && "debug_print string is too long");

    char v2[2048]{};
    vsprintf(v2, fmtp, va);

    va_end(va);

    printf(v2);
    printf("\n");
}
