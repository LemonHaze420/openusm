#include "nslinit.h"

#include "func_wrapper.h"
#include "nslbank.h"

#include <variables.h>

int nsdInit()
{
#if STANDALONE_SYSTEM
    return g_directSound != nullptr;
#else
    return CDECL_CALL(0x0079CE20);
#endif
}

void nslStart(void *work_memory)
{
#if STANDALONE_SYSTEM
    (void)work_memory;
    auto *master = nsl_GetMaster();
    auto *listener = nsl_GetListener();
    master[2] = 1.0f;
    master[3] = 1.0f;
    master[6] = 0.0f;
    master[7] = 0.0f;
    listener[2] = 1.0f;
    listener[3] = 1.0f;
    listener[6] = 0.0f;
    listener[7] = 0.0f;
    nsdInit();
#else
    CDECL_CALL(0x0079A8C0, work_memory);
#endif
}
