#include "parking_marker.h"

#include "common.h"

VALIDATE_SIZE(parking_marker, 0x68);

parking_marker::parking_marker(const string_hash &a2, uint32_t a3) : marker(a2, a3)
{
}
