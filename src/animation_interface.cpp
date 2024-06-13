#include "animation_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(animation_interface, 0x14);

animation_interface::animation_interface(conglomerate *a2) : conglomerate_interface(a2)
{
    this->m_vtbl = 0x00883CE4;
}

void animation_interface::_un_mash(generic_mash_header *a2, void *a3, int , generic_mash_data_ptrs *a5)
{
    TRACE("animation_interface::un_mash");

    this->my_conglomerate = static_cast<conglomerate *>(a3);
    this->dynamic = false;
    this->field_C.custom_un_mash(a2, &this->field_C, a5, this);
}

void animation_interface::release_ifc()
{
    THISCALL(0x004D3C60, this);
}

void animation_interface_patch()
{
    {
        FUNC_ADDRESS(address, &animation_interface::_un_mash);
        set_vfunc(0x00883D00, address);
    }
}
