#pragma once

#include "info_node.h"
#include <array>

namespace ai::native_inode {



template<class T, unsigned Type, unsigned Parent = 537, unsigned Slots = 12>
struct table : std::array<void *, Slots> {
    static void *__fastcall destroy(T *self, void *, unsigned flags)
    {
        self->~T();
        if (flags & 1)
            mash_virtual_base::operator delete(self, sizeof(T));
        return self;
    }
    static unsigned __fastcall type(T *, void *) { return Type; }
    static bool __fastcall subclass(T *, void *, unsigned value)
    {
        return value == Parent || value == 537 || value == 573;
    }
    static int __fastcall size(T *, void *) { return sizeof(T); }

    table()
    {
        auto **base = static_cast<void **>(info_node::native_vtable());
        for (unsigned i = 0; i < 12; ++i)
            (*this)[i] = base[i];
        (*this)[2] = reinterpret_cast<void *>(&destroy);
        (*this)[3] = reinterpret_cast<void *>(&type);
        (*this)[4] = reinterpret_cast<void *>(&subclass);
        (*this)[11] = reinterpret_cast<void *>(&size);
    }
};

inline bool __fastcall always_advance(info_node *, void *) { return true; }

}
