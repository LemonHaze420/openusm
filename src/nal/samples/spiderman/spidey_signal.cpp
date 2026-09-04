#include "spidey_signal.h"

#include "common.h"
#include "variables.h"

VALIDATE_SIZE(spideySignal, 0x4);

#if !STANDALONE_SYSTEM

spideySignal &Component_spideySignal = var<spideySignal>(0x0091F990);

#else

spideySignal &Component_spideySignal = []() -> auto & {
    static spideySignal g_Component_spideySignal{};
    return g_Component_spideySignal;
}();

#endif

spideySignal::spideySignal()
{
    if constexpr (1) {
        static void *g_vtbl[]{func_address(&_GetType), nullptr, nullptr, nullptr, func_address(&_Process)};
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x00881198;
    }
}
