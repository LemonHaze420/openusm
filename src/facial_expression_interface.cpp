#include "facial_expression_interface.h"

#include "common.h"
#include "func_wrapper.h"
#include "parse_generic_mash.h"
#include "trace.h"

VALIDATE_SIZE(facial_expression_interface, 0x7C);

namespace expression_duration_defaults {
constexpr float table[3]{0.30000001, 0.40000001, 0.30000001};
}

facial_expression_interface::facial_expression_interface(actor *a1) : actor_interface(a1)
{
    if constexpr (0) {
        this->m_vtbl = 0x008A5C20;

        this->add_to_facial_expression_ifc_list();
        this->owner_actor = a1;
        this->field_30 = false;

        this->field_78 = 0;
        this->field_70 = -1;
        this->field_74 = -1;

        this->field_34[2].field_4[0] = expression_duration_defaults::table[0];
        this->field_34[2].field_4[1] = expression_duration_defaults::table[1];
        this->field_34[2].field_4[2] = expression_duration_defaults::table[2];
    } else {
        THISCALL(0x006D1670, this, a1);
    }
}

bool facial_expression_interface::is_dynamic() const
{
    return this->dynamic;
}

void facial_expression_interface::add_to_facial_expression_ifc_list()
{
    THISCALL(0x006CFE50, this);
}

void facial_expression_interface::frame_advance_all_facial_expression_ifc(Float a1)
{
    CDECL_CALL(0x006C9940, a1);
}

void facial_expression_interface::un_mash(generic_mash_header *, actor *a3, void *, generic_mash_data_ptrs *)
{
    this->my_actor = a3;
    this->dynamic = false;
    this->add_to_facial_expression_ifc_list();
    this->owner_actor = a3;
}

const char *facial_expression_interface::get_ifc_type_str() const
{
    return "facial_expression";
}

void facial_expression_interface_patch()
{
    REDIRECT(0x004FC217, func_address(&facial_expression_interface::un_mash));
}
