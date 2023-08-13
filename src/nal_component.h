#pragma once

#include <cstdint>

namespace nalGeneric {
struct nalComponentInfo;
}

template<class T0, class T1, class T2>
struct nalComponent : T0, T1 {
    std::intptr_t m_vtbl;

    nalComponent() {
        static struct {
            char field_0[0x10];
            void (nalComponent::*Process)(const nalGeneric::nalComponentInfo *, void *&, void *&);
        } vtbl;

        vtbl.Process = &nalComponent::Process;

        m_vtbl = (int) &vtbl;
    }

    void Process(const nalGeneric::nalComponentInfo *a1, void *&a2, void *&);
};
