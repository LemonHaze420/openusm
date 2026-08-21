#pragma once

#include "log.h"

inline decltype(auto) getReturnAddress()
{
    return bit_cast<unsigned long>(__builtin_return_address(0));
}
