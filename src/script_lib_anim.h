#pragma once

#include "script_library_class.h"

struct slc_anim_t : script_library_class {
    slc_anim_t(const char *n, int sz, const char *p = nullptr) : script_library_class(n, sz, p, false)
    {
        m_vtbl = CAST(m_vtbl, 0x0089AAB0);
    }
};

extern slc_anim_t *slc_anim;

extern void register_anim_lib();
