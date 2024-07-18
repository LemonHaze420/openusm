#pragma once

#include <cstdint>

enum _nlPlatformEnum {
    NL_PLATFORM_PS2,
    NL_PLATFORM_XBOX,
    NL_PLATFORM_GAMECUBE,
    NL_PLATFORM_PC,
    NL_PLATFORM_Z 
};

extern _nlPlatformEnum & g_platform;


