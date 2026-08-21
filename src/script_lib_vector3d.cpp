#include "script_lib_vector3d.h"

#include "utility.h"
#include "vm_stack.h"

slf__angle_between__vector3d__vector3d__t::slf__angle_between__vector3d__vector3d__t(const char *a3) : function(a3)
{
    m_vtbl = CAST(m_vtbl, 0x0089BA50);
    FUNC_ADDRESS(address, &slf__angle_between__vector3d__vector3d__t::operator());
    m_vtbl->__cl = CAST(m_vtbl->__cl, address);
}


bool slf__angle_between__vector3d__vector3d__t::operator()(vm_stack &stack, script_library_class::function::entry_t)
{
    SLF_PARMS;

    auto *v4 = parms;
    auto result = compute_angle_between_vectors(v4->v0, v4->v1);

    SLF_RETURN;
    SLF_DONE;
}
