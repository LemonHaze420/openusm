#include "damage_inode.h"

#include <algorithm>
#include <array>

#include "actor.h"
#include "common.h"
#include "damage_interface.h"
#include "wds.h"

namespace ai {
VALIDATE_SIZE(damage_inode, 0x24);

namespace {
void *__fastcall damage_delete(damage_inode *self, void *, unsigned flags)
{
    self->~damage_inode();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(damage_inode));
    return self;
}
unsigned __fastcall damage_type(damage_inode *, void *)
{
    return 343;
}
bool __fastcall damage_subclass(damage_inode *, void *, unsigned type)
{
    return type == 537 || type == 573;
}
bool __fastcall damage_needs_advance(damage_inode *, void *)
{
    return true;
}
void __fastcall damage_activate(damage_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
int __fastcall damage_size(damage_inode *, void *)
{
    return sizeof(damage_inode);
}
}  // namespace

void *damage_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 12> result;
        std::copy_n(static_cast<void **>(info_node::native_vtable()), result.size(), result.data());
        result[2] = reinterpret_cast<void *>(&damage_delete);
        result[3] = reinterpret_cast<void *>(&damage_type);
        result[4] = reinterpret_cast<void *>(&damage_subclass);
        result[6] = reinterpret_cast<void *>(&damage_needs_advance);
        result[8] = reinterpret_cast<void *>(&damage_activate);
        result[11] = reinterpret_cast<void *>(&damage_size);
        return result;
    }();
    return table.data();
}

damage_inode::damage_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[343]);
}

damage_inode::damage_inode(from_mash_in_place_constructor *tag) : info_node(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[343]);
}

void damage_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    field_20 = g_world_ptr->time_manager.field_8;
}

void damage_inode::apply_damage(actor *source, int amount, vector3d direction, const string_hash &attack,
                                const string_hash &category, const string_hash &reaction, bool force_reaction,
                                int combo_type, bool skip_combat)
{
    if ((field_C->field_8 & 0x4000) != 0)
        return;

    vector3d position = field_C->get_abs_position();
    if (source != nullptr)
        position = source->get_abs_position();
    field_C->damage_ifc()->apply_damage(source,
                                        static_cast<float>(amount),
                                        1,
                                        position,
                                        direction,
                                        0,
                                        attack,
                                        category,
                                        reaction,
                                        force_reaction,
                                        ZEROVEC,
                                        combo_type,
                                        skip_combat);
    field_1C = g_world_ptr->time_manager.field_C;
    field_20 = g_world_ptr->time_manager.field_8;
}

void damage_inode::apply_forced_damage(int amount, vector3d direction, const string_hash &attack, bool force_reaction)
{
    if ((field_C->field_8 & 0x4000) != 0)
        return;

    const string_hash category{0};
    const string_hash reaction{0};
    field_C->damage_ifc()->apply_damage(nullptr,
                                        static_cast<float>(amount),
                                        2,
                                        ZEROVEC,
                                        direction,
                                        0,
                                        attack,
                                        category,
                                        reaction,
                                        force_reaction,
                                        ZEROVEC,
                                        17,
                                        false);
    field_1C = g_world_ptr->time_manager.field_C;
    field_20 = g_world_ptr->time_manager.field_8;
}

void damage_inode::apply_subdue(entity *source, float amount, [[maybe_unused]] vector3d direction)
{
    if ((field_C->field_8 & 0x4000) != 0)
        return;

    field_C->damage_ifc()->apply_subdue(source, amount);
    field_1C = g_world_ptr->time_manager.field_C;
    field_20 = g_world_ptr->time_manager.field_8;
}

void damage_inode::set_subdue([[maybe_unused]] entity *source, float amount, [[maybe_unused]] vector3d direction)
{
    if ((field_C->field_8 & 0x4000) != 0)
        return;


    field_C->damage_ifc()->field_21C.sub_48BFB0(amount);
    field_1C = g_world_ptr->time_manager.field_C;
    field_20 = g_world_ptr->time_manager.field_8;
}
}  // namespace ai
