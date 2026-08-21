#pragma once

#include <cstdint>

struct mContainer {
    mContainer();

    mContainer(int) {}

    mContainer(void *) {}

    void initialize(int a2)
    {
        if (!a2) {
            clear();
        }
    }

    void clear()
    {
        field_0 = 0;
        m_size = 0;
    }

    auto size() const
    {
        return this->m_size;
    }

    int field_0;
    size_t m_size;
};
