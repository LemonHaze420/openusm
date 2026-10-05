#include "weapon_inode.h"

#include "base_ai_core.h"
#include "combo_system.h"
#include "common.h"
#include "core_ai_resource.h"
#include "event.h"
#include "event_manager.h"
#include "memory.h"
#include "native_info_node_table.h"
#include "vtbl.h"

#include <algorithm>

namespace ai {

VALIDATE_SIZE(weapon_inode, 0x38);
string_hash weapon_inode::default_id{"WEAPON"};

namespace {
void __fastcall weapon_mashed_destruct(weapon_inode *self, void *) { self->destruct_mashed_class(); }
void __fastcall weapon_unmash(weapon_inode *self, void *, mash_info_struct *info, void *owner)
{
    self->_unmash(info, owner);
}
void __fastcall weapon_activate(weapon_inode *self, void *, ai_core *core) { self->activate(core); }
void __fastcall weapon_deactivate(weapon_inode *self, void *) { self->destroy_weapons(); }

void weapon_show(event *, entity_base_vhandle, void *context)
{
    if (context) {
        auto handle = static_cast<weapon_inode *>(context)->get_weapon_handle(0);
        if (auto *weapon = handle.get_volatile_ptr()) {
            auto call = reinterpret_cast<void (__fastcall *)(handheld_item *, void *)>(get_vfunc(weapon->m_vtbl, 0x2D0));
            call(weapon, nullptr);
        }
    }
}
void weapon_hide(event *, entity_base_vhandle, void *context)
{
    if (context) {
        auto handle = static_cast<weapon_inode *>(context)->get_weapon_handle(0);
        if (auto *weapon = handle.get_volatile_ptr()) {
            auto call = reinterpret_cast<void (__fastcall *)(handheld_item *, void *)>(get_vfunc(weapon->m_vtbl, 0x2CC));
            call(weapon, nullptr);
        }
    }
}
}

void *weapon_inode::native_vtable()
{


    static auto table = [] {
        native_inode::table<weapon_inode, 410> result;
        result[0] = reinterpret_cast<void *>(&weapon_mashed_destruct);
        result[1] = reinterpret_cast<void *>(&weapon_unmash);
        result[8] = reinterpret_cast<void *>(&weapon_activate);
        result[9] = reinterpret_cast<void *>(&weapon_deactivate);
        return result;
    }();
    return table.data();
}

weapon_inode::weapon_inode() : info_node(), field_1C()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

weapon_inode::weapon_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), field_1C(constructor)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(native_vtable());
}

weapon_inode::~weapon_inode()
{
    clear_weapons();
}

void weapon_inode::_unmash(mash_info_struct *info, void *owner)
{
    info_node::_unmash(info, owner);
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    field_1C.m_size = *reinterpret_cast<int *>(info->read_from_buffer(mash::SHARED_BUFFER, 4, 4));
#endif
    if (field_1C.m_data) {
        field_1C.m_data = reinterpret_cast<weapon_instance **>(info->read_from_buffer(4 * field_1C.m_size, 4));
        for (int i = 0; i < field_1C.m_size; ++i)
            field_1C.m_data[i] = reinterpret_cast<weapon_instance *>(info->read_from_buffer(sizeof(weapon_instance), 4));
    }
    field_1C.field_0 = static_cast<int>(info->mash_image_ptr[0] + info->buffer_size_used[0] -
                                      reinterpret_cast<unsigned char *>(&field_1C));
}

void weapon_inode::destruct_mashed_class()
{
    clear_weapons();
    field_1C.mContainer_base::destruct_mashed_class();
    info_node::_destruct_mashed_class();
}

void weapon_inode::clear_weapons()
{
    if (field_1C.field_10) {
        for (int i = 0; i < field_1C.m_size; ++i) {
            auto *weapon = field_1C.m_data[i];
            if (field_1C.is_pointer_in_mash_image(weapon))
                weapon->destruct_mashed_class();
            else
                delete weapon;
            field_1C.m_data[i] = nullptr;
        }
    }
    if (!field_1C.is_pointer_in_mash_image(field_1C.m_data))
        mem_dealloc(field_1C.m_data, sizeof(weapon_instance *) * field_1C.m_max_size);
    field_1C.m_data = nullptr;
    field_1C.m_max_size = 0;
    field_1C.m_size = 0;
    field_1C.field_0 = 0;
}

void weapon_inode::activate(ai_core *core)
{
    info_node::_activate(core);
    create_weapons();
}

void weapon_inode::create_weapons()
{
    auto *combos = field_8->field_6C->field_10;
    const int count = combos->get_num_weapons();
    for (int i = 0; i < count; ++i) {
        auto *weapon = new weapon_instance(combos->get_weapon(i), field_C);
        if (field_1C.m_size == field_1C.m_max_size || field_1C.is_pointer_in_mash_image(field_1C.m_data)) {
            const int capacity = 8 * (field_1C.m_size / 8) + 8;
            auto **data = static_cast<weapon_instance **>(mem_alloc(sizeof(weapon_instance *) * capacity));
            std::copy_n(field_1C.m_data, field_1C.m_size, data);
            if (!field_1C.is_pointer_in_mash_image(field_1C.m_data))
                mem_dealloc(field_1C.m_data, sizeof(weapon_instance *) * field_1C.m_max_size);
            field_1C.m_data = data;
            field_1C.m_max_size = capacity;
        }
        field_1C.m_data[field_1C.m_size++] = weapon;
    }
    field_30 = event_manager::add_callback(event::WEAPON_SHOW, field_C->my_handle, weapon_show, this, false);
    field_34 = event_manager::add_callback(event::WEAPON_HIDE, field_C->my_handle, weapon_hide, this, false);
}

void weapon_inode::destroy_weapons()
{
    event_manager::remove_callback(field_30, event::WEAPON_SHOW, field_C->my_handle);
    event_manager::remove_callback(field_34, event::WEAPON_HIDE, field_C->my_handle);
    clear_weapons();
}

vhandle_type<handheld_item> weapon_inode::get_weapon_handle(uint16_t index) const
{
    return field_1C.m_data[index]->handle;
}

} // namespace ai
