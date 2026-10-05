#include "script_data_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"
#include "parse_generic_mash.h"
#include <cstring>
#include <new>

VALIDATE_SIZE(script_data_interface, 0x8C);

void script_data_interface::_un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5)
{
    TRACE("script_data_interface::un_mash");

#if STANDALONE_SYSTEM

    my_conglomerate = static_cast<conglomerate *>(a3);
    dynamic = false;
    new (&field_34) mString{};
    new (&field_44) mString{};
    new (&field_54) mString{};
    new (&field_64) mString{};
    std::memcpy(field_C, a5->get_from_shared<char>(sizeof(field_C)), sizeof(field_C));
    for (auto *text : {&field_34, &field_44, &field_54, &field_64}) {
        a5->rebase_shared(4);
        const auto length = *a5->get_from_shared<uint32_t>();
        *text = a5->get_from_shared<char>(length);
    }
    field_74 = *a5->get_from_shared<vector3d>();
    field_80 = *a5->get_from_shared<vector3d>();

    (void)a2;
    (void)a4;
#else
    THISCALL(0x004BEEF0, this, a2, a3, a4, a5);
#endif
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
