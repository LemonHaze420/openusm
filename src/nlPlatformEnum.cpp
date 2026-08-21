#include "nlPlatformEnum.h"

#include "variable.h"
#include "variables.h"

#if !STANDALONE_SYSTEM
_nlPlatformEnum &g_platform = var<_nlPlatformEnum>(0x0095C1E4);
#else
_nlPlatformEnum &g_platform = []() -> auto & {
    static _nlPlatformEnum g_platform1{};
    return g_platform1;
}();
#endif
