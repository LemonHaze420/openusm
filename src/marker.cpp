#include "marker.h"

#include "common.h"

VALIDATE_SIZE(marker, 0x68);

marker::marker(const string_hash &a2, uint32_t a3) : entity(a2, a3) {}
