#include "script_data_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(script_data_interface, 0x8C);

void script_data_interface::_un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5)
{
    TRACE("script_data_interface::un_mash");

    THISCALL(0x004BEEF0, this, a2, a3, a4, a5);
}

void script_data_interface::release_ifc()
{
    this->field_34.~mString();
    this->field_44.~mString();
    this->field_54.~mString();
    this->field_64.~mString();
}

void script_data_interface_patch()
{
    {
        FUNC_ADDRESS(address, &script_data_interface::_un_mash);
        set_vfunc(0x00882F64, address);
    }
}
