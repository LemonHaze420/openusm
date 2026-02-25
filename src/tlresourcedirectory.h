#pragma once

#include "fixedstring.h"
#include "vtbl.h"

#include <cstdint>

template<typename T0, typename T1>
struct tlResourceDirectory {
    std::intptr_t m_vtbl;

    struct Iterator {
        std::intptr_t m_vtbl;

        //virtual
        void finalize(bool a1) {
            void (__fastcall *func)(void *, void *edx, bool) = CAST(func, get_vfunc(this->m_vtbl, 0x0));
            func(this, nullptr, a1);
        }

        //virtual
        void reset() { // = 0;
            void (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x4));
            func(this);
        }

        //virtual
        bool operator()() { // = 0;
            bool (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x8));
            return func(this);
        }

        //virtual
        T0 * operator*() { // = 0;
            T0 * (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0xC));
            return func(this);
        }

        //virtual
        void operator++() { // = 0;
            void (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x10));
            func(this);
        }
    };

    /* virtual */ ~tlResourceDirectory() = default;

    /* virtual */ const char *DirectoryName() {
        return "Unnamed";
    }

    T0 * _Find(unsigned int) {
        return nullptr;
    }

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

    //virtual
    bool Del(T0 *a2) { // = 0;
        bool (__fastcall *func)(void *, void *edx, T0 *) = CAST(func, get_vfunc(this->m_vtbl, 0x14));
        return func(this, nullptr, a2);
    }

    //virtual
    Iterator * Enumerate()
    {
        Iterator * (__fastcall *func)(void *) = CAST(func, get_vfunc(this->m_vtbl, 0x18));
        return func(this);
    }

    void _ReleaseAll(bool a2, bool a3, int a4)
    {
        auto *iter = this->Enumerate();
        for ( int i = 1; i <= a4; ++i )
        {
            iter->reset();
            while ((*iter)())
            {
                auto v7 = *(*iter);
                auto v8 = this->Release(v7, i, a3);
                if ( v8 > 0 ) {
                    ++(*iter);
                } else {
                    iter->reset();
                }

                if ( v8 == 0 && a2 ) {
                    tlFixedString v10 {};
                    auto v9 = v10.to_string();
                    auto directoryName = this->DirectoryName();
                    sp_log("Resource %s: %s was not released.\n", directoryName, v9);
                }
            }
        }

        if ( iter != nullptr ) {
            iter->finalize(true);
        }
    }

    //virtual
    void ReleaseAll(bool a2, bool a3, int a4)
    {
        void (__fastcall *func)(void *, void *edx, bool, bool, int) = CAST(func, get_vfunc(this->m_vtbl, 0x1C));
        return func(this, nullptr, a2, a3, a4);
    }

    //virtual
    T0 * Load(const T1 &a2) {
        return StandardLoad(a2);
    }

    int _Release(T0 *a2, int a3, bool a4) {
        return this->StandardRelease(a2, a3, a4);
    }

    //virtual
    int Release(T0 *a2, int a3, bool a4) {
        int (__fastcall *func)(void *, void *edx, T0 *, int, bool) = CAST(func, get_vfunc(this->m_vtbl, 0x28));
        return func(this, nullptr, a2, a3, a4);
    }

    T0 *StandardLoad(const T1 &);

    int StandardRelease(T0 *a2, int a3, bool a4);
};

extern void tlResourceDirectory_patch();
