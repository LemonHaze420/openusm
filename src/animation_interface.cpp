#include "animation_interface.h"

#include "common.h"
#include "anim_map_ptr_entry.h"
#include "entity_mash.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(animation_interface, 0x14);

animation_interface::animation_interface(conglomerate *a2) : conglomerate_interface(a2)
{
#if STANDALONE_SYSTEM
    construct_v_table_lookup();
    m_vtbl = ifc_v_table_lookup[0];
#else
    m_vtbl = 0x00883CE4;
#endif
}

animation_interface::~animation_interface()
{
    if (!field_C.from_mash())
        delete[] field_C.m_data;
    field_C.m_data = nullptr;
    field_C.m_size = 0;
    my_conglomerate = nullptr;
}

void animation_interface::_un_mash(generic_mash_header *a2, void *a3, int, generic_mash_data_ptrs *a5)
{
    TRACE("animation_interface::un_mash");

    this->my_conglomerate = static_cast<conglomerate *>(a3);
    this->dynamic = false;
    this->field_C.custom_un_mash(a2, &this->field_C, a5, this);
}

void animation_interface::release_ifc()
{
    int local_references = 1;
    int *references = field_C.is_shared() ? reinterpret_cast<int *>(field_C.m_data) - 1 : &local_references;
    if (--*references == 0) {
        for (auto &entry : field_C) {
            if (entry.field_8.is_shared())
                --*(reinterpret_cast<int *>(entry.field_8.m_data) - 1);
        }
    }
}

void animation_interface_patch()
{
    {
        FUNC_ADDRESS(address, &animation_interface::_un_mash);
        set_vfunc(0x00883D00, address);
    }
}
