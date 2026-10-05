#include "advanced_entity_ptrs.h"

#include "common.h"
#include "item.h"
#include "memory.h"
#include "parse_generic_mash.h"
#include <func_wrapper.h>
#include "resource_key.h"
#include "resource_manager.h"
#include "resource_pack_slot.h"
#include "script.h"
#include "script_access.h"
#include "script_manager.h"
#include "wds.h"
#include "vtbl.h"
#include <cassert>

VALIDATE_SIZE(movement_info, 0x58);
VALIDATE_SIZE(advanced_entity_ptrs, 0x14);
VALIDATE_OFFSET(advanced_entity_ptrs, mi, 0xC);

advanced_entity_ptrs::~advanced_entity_ptrs()
{
    if (field_8 != nullptr)
        mem_dealloc(field_8, sizeof(*field_8));
    field_8 = nullptr;
    if (mi != nullptr)
        mem_dealloc(mi, sizeof(*mi));
    mi = nullptr;
    if (coninfo != nullptr) {
        for (const auto &handle : coninfo->items) {
            auto *value = handle.get_volatile_ptr();
            if (value != nullptr)
                g_world_ptr->ent_mgr.destroy_entity(value);
        }
        coninfo->~coninfo_t();
        mem_dealloc(coninfo, sizeof(*coninfo));
        coninfo = nullptr;
    }
    if (ignore_col_ents != nullptr) {
        ignore_col_ents->~vector();
        mem_dealloc(ignore_col_ents, sizeof(*ignore_col_ents));
        ignore_col_ents = nullptr;
    }
    if (my_script != nullptr)
        script_manager::release_actor_script(my_script);
}

void advanced_entity_ptrs::un_mash(generic_mash_header *header,
                                    actor *arg4,
                                    void *object,
                                    generic_mash_data_ptrs *data)
{
#if STANDALONE_SYSTEM
    (void)object;
    this->coninfo = nullptr;
    this->ignore_col_ents = nullptr;
    this->field_8 = nullptr;
    this->mi = nullptr;
    this->my_script = nullptr;


    const resource_key script_key = *data->get_from_shared<resource_key>();
    if (script_key.is_set()) {
        resource_pack_slot *slot = nullptr;
        resource_manager::get_resource(script_key, nullptr, &slot);
        assert(slot != nullptr);
        script_manager::load(script_key, 0, slot, slot->get_name_key());
        script_manager::link();
        auto *script_object = script_manager::find_object(
            script_key, string_hash{"ENX_MASTER_OBJECT"}, slot->get_name_key());
        this->my_script = script::create_instance(string_hash{"__enx"}, script_object);
        script::push_arg(arg4);
        script::exec_thread(false);
    }

    if (header->is_flagged(0x4000)) {
        const vector3d value = *data->get<vector3d>();
        auto callback = reinterpret_cast<void(__fastcall *)(actor *, void *, const vector3d &)>(
            get_vfunc(arg4->m_vtbl, 0x1D0));
        callback(arg4, nullptr, value);
    }

    if (header->is_flagged(0x2000)) {
        const auto count = *data->get_from_shared<uint16_t>();
        for (unsigned index = 0; index < count; ++index) {
            const resource_key item_key = *data->get_from_shared<resource_key>();
            const resource_key visual_key = *data->get_from_shared<resource_key>();
            const auto quantity = *data->get_from_shared<uint32_t>();
            const auto flags = *data->get_from_shared<uint8_t>();
            if (!item_key.is_set())
                continue;
            auto *inventory_item = g_world_ptr->ent_mgr.create_and_add_entity_or_subclass(
                item_key.m_hash, make_unique_entity_id(), po_identity_matrix, mString{""}, 0x81, nullptr);
            auto set_quantity = reinterpret_cast<void(__fastcall *)(entity *, void *, uint32_t)>(
                get_vfunc(inventory_item->m_vtbl, 0x2A4));
            set_quantity(inventory_item, nullptr, quantity);
            if (flags != 0) {

                auto **movement = reinterpret_cast<uint8_t **>(
                    reinterpret_cast<uint8_t *>(inventory_item) + 0x100);
                *reinterpret_cast<resource_key *>(*movement + 0x10) = visual_key;
                auto &state = *reinterpret_cast<uint32_t *>(
                    reinterpret_cast<uint8_t *>(inventory_item) + 0x10C);
                state = (state & ~0x36u) | (flags & 6u) |
                    ((flags & 8u) << 1) | (flags & 0x20u);
                data->rebase_shared(16);
                data->rebase_shared(4);
                auto *new_movement = data->get_from_shared<uint8_t>(0x60);
                new_movement[0x5F] &= 0x7F;
                if (((*movement)[0x5F] & 0x80) != 0)
                    ::operator delete(*movement);
                *movement = new_movement;
                auto set_owner = reinterpret_cast<void(__fastcall *)(entity *, void *, actor *)>(
                    get_vfunc(inventory_item->m_vtbl, 0x2E0));
                set_owner(inventory_item, nullptr, arg4);
                auto set_inventory_state = [&](int offset) {
                    auto callback = reinterpret_cast<void(__fastcall *)(entity *, void *, bool)>(
                        get_vfunc(inventory_item->m_vtbl, offset));
                    callback(inventory_item, nullptr, true);
                };
                set_inventory_state((flags & 0x10u) != 0 ? 0x2C8 : 0x2C4);
                set_inventory_state((state & 1u) != 0 ? 0x2C4 : 0x2C8);
                set_inventory_state((state & 1u) != 0 ? 0x2C8 : 0x2C4);
            }
            arg4->add_item(inventory_item->get_my_handle().get_goodies(), true);
        }
    }

#else
    THISCALL(0x004CFCE0, this, header, arg4, object, data);
#endif
}
