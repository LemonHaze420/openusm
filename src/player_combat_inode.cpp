#include "player_combat_inode.h"

#include "common.h"
#include "script_manager.h"
#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(player_combat_inode, 0x330);

namespace {
void *__fastcall player_combat_delete(player_combat_inode *self, void *, unsigned flags)
{
    self->~player_combat_inode();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
unsigned __fastcall player_combat_type(player_combat_inode *, void *)
{
    return 346;
}
bool __fastcall player_combat_subclass(player_combat_inode *, void *, unsigned type)
{
    return type == 342 || type == 537 || type == 573;
}
void __fastcall player_combat_advance(player_combat_inode *self, void *, Float delta)
{
    self->_frame_advance(delta);
}
void __fastcall player_combat_activate(player_combat_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
int __fastcall player_combat_size(player_combat_inode *, void *)
{
    return sizeof(player_combat_inode);
}
void __fastcall player_combat_damage(player_combat_inode *self, void *, float amount)
{
    self->field_328 = amount;
}
int __fastcall player_combat_level(player_combat_inode *self, void *)
{
    return static_cast<int>(*self->field_32C);
}
}  // namespace

void *player_combat_inode::native_vtable()
{
    static auto table = [] {
        std::array<void *, 76> result;
        std::copy_n(static_cast<void **>(combat_inode::native_vtable()), result.size(), result.data());
        result[2] = reinterpret_cast<void *>(&player_combat_delete);
        result[3] = reinterpret_cast<void *>(&player_combat_type);
        result[4] = reinterpret_cast<void *>(&player_combat_subclass);
        result[7] = reinterpret_cast<void *>(&player_combat_advance);
        result[8] = reinterpret_cast<void *>(&player_combat_activate);
        result[11] = reinterpret_cast<void *>(&player_combat_size);
        result[0x118 / 4] = reinterpret_cast<void *>(&player_combat_damage);
        result[0x11C / 4] = reinterpret_cast<void *>(&player_combat_level);
        return result;
    }();
    return table.data();
}

player_combat_inode::player_combat_inode()
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[346]);
}

player_combat_inode::player_combat_inode(from_mash_in_place_constructor *tag) : combat_inode(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[346]);
}

void player_combat_inode::_frame_advance(Float delta)
{
    combat_inode::_frame_advance(delta);
    field_328 = 0.0f;
}

void player_combat_inode::_activate(ai_core *a2)
{
    combat_inode::_activate(a2);
    this->field_328 = 0.0f;
    mString v1{"gv_combat_level"};
    this->field_32C = static_cast<float *>(script_manager::get_game_var_address(v1, nullptr, nullptr));
}

}  // namespace ai
