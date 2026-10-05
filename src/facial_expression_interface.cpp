#include "facial_expression_interface.h"

#include "common.h"
#include "entity_mash.h"
#include <algorithm>
#include "func_wrapper.h"
#include "parse_generic_mash.h"
#include "trace.h"

VALIDATE_SIZE(facial_expression_interface, 0x7C);

namespace expression_duration_defaults {
constexpr float table[3]{0.30000001, 0.40000001, 0.30000001};
}

facial_expression_interface::facial_expression_interface(actor *a1) : actor_interface(a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        construct_v_table_lookup();
        this->m_vtbl = ifc_v_table_lookup[4];

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

facial_expression_interface::~facial_expression_interface()
{
    release_ifc();
}

void facial_expression_interface::release_ifc()
{
    auto *&interfaces = all_facial_expression_interfaces();
    auto found = std::find(interfaces->begin(), interfaces->end(), this);
    if (found != interfaces->end())
        interfaces->erase(found);
    if (interfaces->empty()) {
        delete interfaces;
        interfaces = nullptr;
    }
}

bool facial_expression_interface::is_dynamic() const
{
    return this->dynamic;
}

void facial_expression_interface::add_to_facial_expression_ifc_list()
{
#if STANDALONE_SYSTEM
    auto *&interfaces = all_facial_expression_interfaces();
    if (interfaces == nullptr) {
        interfaces = new _std::vector<facial_expression_interface *>{};
    }
    interfaces->push_back(this);
#else
    THISCALL(0x006CFE50, this);
#endif
}

void facial_expression_interface::frame_advance_all_facial_expression_ifc(Float a1)
{
    auto *interfaces = all_facial_expression_interfaces();
    if (interfaces == nullptr) {
        return;
    }
    for (auto *interface_ptr : *interfaces) {
        if (interface_ptr != nullptr) {
            interface_ptr->frame_advance(a1);
        }
    }
}

void facial_expression_interface::frame_advance(Float elapsed)
{
#if STANDALONE_SYSTEM
    auto advance_slot = [elapsed](auto &slot) {
        if (slot.field_10 > 0.0f) {
            slot.field_10 -= elapsed.value;
        }
        return slot.field_10 > 0.0f;
    };
    if (field_70 != -1 && !advance_slot(field_34[field_70])) {
        field_70 = -1;
    }
    if (field_74 != -1 && !advance_slot(field_34[field_74])) {
        field_74 = -1;
    }
    if (field_70 == -1) {
        field_70 = field_74;
        field_74 = -1;
    }
    if (field_70 == -1) {
        field_30 = false;
    }
#else
    THISCALL(0x006C10C0, this, elapsed);
#endif
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
