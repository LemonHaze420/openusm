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
    m_vtbl = nalNativeStreamTable(nalNativeStreamEncoding::Signal);
}
