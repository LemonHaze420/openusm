#include "nal_generic_component.h"

#include "common.h"
#include "variables.h"

#if !STANDALONE_SYSTEM
nalComponentRLE8Int1 &Component_nalComponentRLE8Int1 = var<nalComponentRLE8Int1>(0x00946A90);
#else
nalComponentRLE8Int1 &Component_nalComponentRLE8Int1 = []() -> auto & {
    static nalComponentRLE8Int1 g_Component_nalComponentRLE8Int1{};
    return g_Component_nalComponentRLE8Int1;
}();
#endif

nalComponentRLE8Int1::nalComponentRLE8Int1()
{
    if constexpr (1) {
        static void *g_vtbl[]{func_address(&_GetType), nullptr, nullptr, nullptr, func_address(&_Process)};
        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        this->m_vtbl = 0x008BD4B8;
    }
}
