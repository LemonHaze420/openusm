#pragma once

#include <cstdint>

#include "vtbl.h"

namespace nalGeneric {
struct nalComponentInfo;
}

template<class T0, class T1, class T2>
struct nalComponent : T0, T1 {

    nalComponent();

    void _Process(const nalGeneric::nalComponentInfo *a1, void *&a2, void *&);
};
