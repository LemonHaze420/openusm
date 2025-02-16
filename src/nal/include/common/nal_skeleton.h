#pragma once

#include <cstdint>

#include <fixedstring.h>

struct nalBaseSkeleton {
    std::intptr_t m_vtbl;
    int Version;
    tlFixedString field_8;
    tlFixedString field_28;

    const tlFixedString & GetName() const
    {
        return this->field_8;
    }

    const tlFixedString & GetAnimTypeName() const
    {
        return this->field_28;
    }

};

//0x0078DC80
extern void *nalConstructSkeleton(void *a1);
