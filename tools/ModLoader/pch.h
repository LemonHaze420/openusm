#ifndef PCH_H
#define PCH_H

#include "framework.h"

#define PLATFORM_PS2    0
#define PLATFORM_XBOX   1
#define PLATFORM_GC     2
#define PLATFORM_PC     3

#ifndef PLATFORM
#define PLATFORM        PLATFORM_PC
#else
#if defined(PLATFORM_PS2)
#define PLATFORM        PLATFORM_PS2
#elif defined(PLATFORM_XBOX)
#define PLATFORM        PLATFORM_XBOX
#elif defined(PLATFORM_GC)
#define PLATFORM        PLATFORM_GC
#elif defined(PLATFORM_PC)
#define PLATFORM        PLATFORM_PC
#endif

#endif

#endif //PCH_H
