#include "weapon_instance.h"

#include "combo_system_weapon.h"
#include "common.h"
#include "mstring.h"
#include "oldmath_po.h"
#include "resource_manager.h"
#include "vtbl.h"
#include "wds.h"

namespace ai {

VALIDATE_SIZE(weapon_instance, 0x8);

weapon_instance::weapon_instance(from_mash_in_place_constructor *)
{
    handle.field_0 = INVALID_HANDLE;
    initialize(mash::FROM_MASH, nullptr, nullptr);
}

weapon_instance::weapon_instance(const combo_system_weapon *weapon, actor *owner)
{
    handle.field_0 = INVALID_HANDLE;
    initialize(mash::ALLOCATED, weapon, owner);
}


void weapon_instance::initialize(mash::allocation_scope, const combo_system_weapon *weapon, actor *owner)
{
    resource_manager::push_resource_context(owner->m_resource_context);
    auto *created = static_cast<handheld_item *>(g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
        weapon->field_0.m_hash, make_unique_entity_id(), po_identity_matrix, mString{""}, 0x81, nullptr));
    resource_manager::pop_resource_context();
    auto set_owner = reinterpret_cast<void (__fastcall *)(handheld_item *, void *, actor *)>(get_vfunc(created->m_vtbl, 0x2E0));
    set_owner(created, nullptr, owner);
    handle.field_0 = created->my_handle;
    definition = weapon;
    auto add = reinterpret_cast<bool (__fastcall *)(actor *, void *, entity_base_vhandle, bool)>(get_vfunc(owner->m_vtbl, 0x288));
    add(owner, nullptr, created->my_handle, false);
    auto *attachments = reinterpret_cast<string_hash *>(created->field_100);
    attachments[0] = weapon->field_8.m_hash;
    attachments[1] = weapon->field_10.m_hash;
    auto equip = reinterpret_cast<void (__fastcall *)(handheld_item *, void *, bool)>(
        get_vfunc(created->m_vtbl, weapon->field_21 ? 0x2C8 : 0x2C4));
    equip(created, nullptr, true);
}

weapon_instance::~weapon_instance()
{
    destruct_mashed_class();
}


void weapon_instance::destruct_mashed_class()
{
    if (auto *weapon = handle.get_volatile_ptr()) {
        auto get_owner = reinterpret_cast<actor *(__fastcall *)(handheld_item *, void *)>(get_vfunc(weapon->m_vtbl, 0x2DC));
        if (auto *owner = get_owner(weapon, nullptr)) {
            auto remove = reinterpret_cast<void (__fastcall *)(actor *, void *, entity_base_vhandle, bool)>(get_vfunc(owner->m_vtbl, 0x28C));
            remove(owner, nullptr, handle.field_0, false);
        }
        g_world_ptr->ent_mgr.destroy_entity(weapon);
    }
}

} // namespace ai
