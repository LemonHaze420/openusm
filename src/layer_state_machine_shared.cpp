#include "layer_state_machine_shared.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"
#include "trace.h"

namespace als {

VALIDATE_SIZE(layer_state_machine_shared, 0x48);

layer_state_machine_shared::layer_state_machine_shared()
{
    if constexpr (1) {
        static void *g_vtbl[] = {nullptr,
                                 func_address(&_unmash),
                                 nullptr,
                                 func_address(&_get_virtual_type_enum),
                                 nullptr,
                                 func_address(&_is_or_is_subclass_of),
                                 func_address(&_get_layer_id),
                                 func_address(&_get_mash_sizeof),
                                 func_address(&_set_layer_id)};

        this->m_vtbl = CAST(m_vtbl, &g_vtbl);
    } else {
        THISCALL(0x00444850, this);
    }
}

layer_state_machine_shared::layer_state_machine_shared(from_mash_in_place_constructor *a2) : state_machine_shared(a2) {}

void layer_state_machine_shared::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("layer_state_machine_shared::unmash");

    if constexpr (1) {
        state_machine_shared::_unmash(a1, a3);
    } else {
        THISCALL(0x004AB690, this, a1, a3);
    }
}

int layer_state_machine_shared::_get_virtual_type_enum() const
{
    return 483;
}

layer_types layer_state_machine_shared::_get_layer_id() const
{
    TRACE("als::layer_state_machine_shared::get_layer_id");

    return this->field_44;
}

int layer_state_machine_shared::_get_mash_sizeof() const
{
    return sizeof(layer_state_machine_shared);
}

void layer_state_machine_shared::_set_layer_id(layer_types a2)
{
    TRACE("als::layer_state_machine_shared::set_layer_id");

    this->field_44 = a2;
}
}  // namespace als

void als_layer_state_machine_shared_patch()
{
    {
        FUNC_ADDRESS(address, als::layer_state_machine_shared::_unmash);
        set_vfunc(0x0087E3A8, address);
    }

    {
        FUNC_ADDRESS(address, als::layer_state_machine_shared::_set_layer_id);
        set_vfunc(0x0087E3C4, address);
    }
}
