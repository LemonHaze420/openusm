#include "pfx_interface.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(pfx_interface, 0x40);

pfx_interface::pfx_interface()
{

}

void pfx_interface::release_ifc()
{
    THISCALL(0x004D9600, this);
}
