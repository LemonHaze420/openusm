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

    bool is_active() const;
    void set_active(bool active);

    static void initialize_native_vtable(std::intptr_t *table, std::intptr_t destroy, std::intptr_t advance);
};
