#include "rstr.h"

#include <cassert>
#include <cstring>

static constexpr auto RIDICULOUS_SIZE = 4096;

char *rstrcpy(char *a1, const char *src, uint32_t dest_buffer_length)
{
    assert(dest_buffer_length > 0);
    assert(dest_buffer_length <= RIDICULOUS_SIZE);
    assert(strlen(src) < dest_buffer_length);

    strncpy(a1, src, dest_buffer_length);
    a1[dest_buffer_length - 1] = '\0';
    return a1;
}
