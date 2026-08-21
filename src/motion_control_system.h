#pragma once

#include <cstdint>

struct entity;

struct motion_control_system {
    std::intptr_t m_vtbl;
    bool field_4;
    bool field_5;
    entity *m_ent;

    motion_control_system();

    void *operator new(size_t size);
};
