#include "variant_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(variant_interface, 0x58);

variant_interface::variant_interface(conglomerate *a2) : conglomerate_interface(a2)
{
    this->m_vtbl = 0x008835A0;
}

variant_info *variant_interface::get_random_variant()
{
    if constexpr (0)
    {
    }
    else
    {
        return (variant_info *) THISCALL(0x004CAD00, this);
    }
}

void variant_interface::apply_variant(string_hash a2)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (0)
    {}
    else
    {
        THISCALL(0x004E0920, this, a2);
    }
}

void variant_interface::apply_variant(variant_info *info)
{
    TRACE("variant_interface::apply_variant");

    if constexpr (0)
    {}
    else
    {
        THISCALL(0x004DB110, this, info);
    }
}

void variant_interface_patch()
{
    {
        void (variant_interface::* apply_variant)(string_hash) = &variant_interface::apply_variant;
        FUNC_ADDRESS(address, apply_variant);
        REDIRECT(0x0066F402, address);
        REDIRECT(0x0066F372, address);
    }
}
