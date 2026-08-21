#pragma once

#include <cstdint>

struct singleton {
    int m_vtbl;

    singleton() = default;

    //virtual
    ~singleton() = default;

    void *operator new(size_t size);

    void operator delete(void *, size_t);
};
