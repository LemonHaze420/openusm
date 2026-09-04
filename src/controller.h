#pragma once

#include <cstdint>

struct controller {
    std::intptr_t m_vtbl;
    bool field_4;
    bool field_5;

    controller();

    void *operator new(size_t size);

    //0x0055E7C0
    //virtual
    void kill();

    //0x0055E7D0
    //virtual
    void resurrect();

    //virtual
    bool is_controller() const;
};

extern void controller_patch();
