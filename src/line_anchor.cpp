#include "line_anchor.h"

#include "common.h"
#include "utility.h"

VALIDATE_SIZE(line_marker_base, 0x84);

line_anchor::line_anchor(const string_hash &a2, uint32_t a3) : line_marker_base(a2, a3) {}

bool line_anchor::is_a_line_anchor()
{
    return true;
}

void line_anchor_patch()
{
    {
        FUNC_ADDRESS(address, &line_anchor::is_a_line_anchor);
        set_vfunc(0x00886808 + 0xB8, address);
    }
}
