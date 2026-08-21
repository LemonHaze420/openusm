#pragma once

#include <cstdint>

#include <nal_generic.h>
#include "utility.h"
#include "vtbl.h"

template <class T0, class T1, class T2>
struct nalComponent : T0, T1 {
    nalComponent()
    {
        static void *g_vtbl[]{func_address(&T0::_GetType), nullptr, nullptr, nullptr, func_address(&_Process)};

        T0::m_vtbl = CAST(T0::m_vtbl, &g_vtbl);
    }

    void _Process(const nalGeneric::nalComponentInfo *a1, void *&a2, void *&)
    {
        for (int i = 0; i < a1->field_28; ++i) {
            a2 = static_cast<char *>(a2) + 1;
        }
    }
};
