#include "corner_info.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(corner_info, 0xA8);

corner_info::corner_info() {}

corner_info::corner_info(const corner_info &a2)
{
    THISCALL(0x006B7590, this, &a2);
}

void corner_info::clear()
{
    if constexpr (1) {
        assert(0);
    } else {
        THISCALL(0x0048C9D0, this);
    }
}

}  // namespace ai
