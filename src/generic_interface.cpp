#include "generic_interface.h"

#include "common.h"
#include "vtbl.h"

VALIDATE_SIZE(generic_interface, 0x4);

generic_interface::generic_interface() {}

#ifndef TARGET_XBOX
void generic_interface::un_mash(generic_mash_header *a1, void *a2, void *a3, generic_mash_data_ptrs *a4)
{
    if (m_vtbl == 0) {
        return;
    }
    auto *address = get_vfunc(m_vtbl, 0x1C);
    if (address != nullptr) {
        void(__fastcall * func)(void *, int, generic_mash_header *, void *, void *, generic_mash_data_ptrs *) =
            CAST(func, address);
        func(this, 0, a1, a2, a3, a4);
    }
}
#endif
