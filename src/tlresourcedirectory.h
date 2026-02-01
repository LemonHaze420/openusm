#pragma once

#include "vtbl.h"

#include <cstdint>

template<typename T0, typename T1>
struct tlResourceDirectory {
    std::intptr_t m_vtbl;

    struct Iterator {
        std::intptr_t m_vtbl;
    };

    /* virtual */ ~tlResourceDirectory() = default;

    /* virtual */ const char *DirectoryName() {
        return "Unnamed";
    }


    T0 * _Find(unsigned int a2);

    //virtual
    T0 * Find(unsigned int a2) {
        T0 * (__fastcall *func)(void *, void *edx, uint32_t) = CAST(func, get_vfunc(this->m_vtbl, 0x8));
        return func(this, nullptr, a2);
    }

    /* virtual */ T0 * Find(const T1 &a2) { // = 0;
        T0 * (__fastcall *func)(void *, void *edx, const T1 *) = CAST(func, get_vfunc(this->m_vtbl, 0xC));
        return func(this, nullptr, &a2);
    }

    /* virtual */ int Add(T0 *a2) { // = 0;
        int (__fastcall *func)(void *, void *edx, T0 *) = CAST(func, get_vfunc(this->m_vtbl, 0x10));
        return func(this, nullptr, a2);
    }

    /* virtual */ void ReleaseAll(bool a2, bool a3, int a4);

    /* virtual */ T0 *Load(const T1 &);

    /* virtual */ int Release(T0 *a2, int a3, bool a4);

    T0 *StandardLoad(const T1 &);

    int StandardRelease(T0 *a2, int a3, bool a4);
};

extern void tlResourceDirectory_patch();
