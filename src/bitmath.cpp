#include "bitmath.h"

namespace bitmath {

bool is_power_of_2(int a1)
{
    return a1 && (a1 & (a1 - 1)) == 0;
}

int intlog2(unsigned int a1)
{
    int v2 = 0;
    while (a1 > 1) {
        a1 >>= 1;
        ++v2;
    }

    return v2;
}
}  // namespace bitmath
