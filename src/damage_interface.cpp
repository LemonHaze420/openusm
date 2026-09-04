#include "damage_interface.h"

#include "actor.h"
#include "common.h"
#include "func_wrapper.h"
#include "resource_key.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"

VALIDATE_SIZE(damage_info, 0x40u);

VALIDATE_OFFSET(damage_interface, field_FC, 0xFCu);
VALIDATE_SIZE(damage_interface, 0x23Cu);

template <>
void bounded_variable<float>::sub_48BFB0(const float &a2)
{
    this->field_0[0] = a2;
    if (a2 > this->field_0[2]) {
        this->field_0[0] = this->field_0[2];
    }

    if (this->field_0[0] < this->field_0[1]) {
        this->field_0[0] = this->field_0[1];
    }
}

damage_interface::damage_interface(actor *a2)
{
    THISCALL(0x004DE8A0, this, a2);
}

damage_interface::~damage_interface()
{
    THISCALL(0x004D9BF0, this);
}

void damage_interface::remove_from_dmg_ifc_list()
{
    for (auto it = all_damage_interfaces->begin(); it != all_damage_interfaces->end(); ++it) {
        if ((*it) == this) {
            all_damage_interfaces->erase(it);
        }
    }

    if (all_damage_interfaces->empty()) {
        delete all_damage_interfaces;
        all_damage_interfaces = nullptr;
    }

    if (found_damageable != nullptr) {
        delete found_damageable;
        found_damageable = nullptr;
    }
}


bool damage_interface::get_ifc_num(const resource_key &att, float *a3, bool is_log)
{
    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

    return (bool)THISCALL(0x004C8C60, this, &att, a3, is_log);
}

bool damage_interface::set_ifc_num(const resource_key &att, Float a3, bool is_log)
{
    assert(att.get_type() == RESOURCE_KEY_TYPE_IFC_ATTRIBUTE);

    return (bool)THISCALL(0x004CE940, this, &att, a3, is_log);
}

void damage_interface::frame_advance_all_damage_ifc(Float a1)
{
    TRACE("damage_interface::frame_advance_all_damage_ifc");

    if constexpr (1) {
        if (all_damage_interfaces != nullptr && !all_damage_interfaces->empty()) {
            sp_log("%d", all_damage_interfaces->size());
            for (auto &dam : (*all_damage_interfaces)) {
                if (dam != nullptr) {
                    sp_log("%s", dam->field_4->field_10.to_string());

                    dam->frame_advance(a1);
                    void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(dam->m_vtbl, 0x28));
                    func(dam, nullptr, a1);
                }
            }
        }
    } else {
        CDECL_CALL(0x004D1990, a1);
    }
}

void damage_interface::_un_mash(generic_mash_header *header, void *a3, void *a4, generic_mash_data_ptrs *a5)
{
    TRACE("damage_interface::un_mash");

    if constexpr (0) {
    } else {
        THISCALL(0x004D9E20, this, header, a3, a4, a5);
    }
}

void damage_interface::release_ifc()
{
    this->remove_from_dmg_ifc_list();
    if (this->field_1F4 && this->field_1CC != nullptr) {
        operator delete[](this->field_1CC);
    }

    this->field_1CC = nullptr;

    auto v2 = this->field_1F5;
    if (v2 && this->field_1D0 != nullptr) {
        operator delete[](this->field_1D0);
    }

    this->field_1D0 = nullptr;

    this->field_184.release_mem();
}

void damage_interface::frame_advance(Float a3)
{
    TRACE("damage_interface::frame_advance");

    THISCALL(0x004EC4A0, this, a3);
}

void damage_interface_patch()
{
    REDIRECT(0x00558500, damage_interface::frame_advance_all_damage_ifc);

    {
        FUNC_ADDRESS(address, &damage_interface::_un_mash);
        set_vfunc(0x0088398C, address);
    }

    {
        FUNC_ADDRESS(address, &damage_interface::frame_advance);
        set_vfunc(0x00883998, address);
    }
}
