#include "combat_inode.h"

#include "common.h"

#include <vtbl.h>

namespace ai {

VALIDATE_SIZE(combat_inode, 0x328);
VALIDATE_OFFSET(combat_inode, field_68, 0x68);

VALIDATE_SIZE(combat_inode::incoming_move, 0x94);

combat_inode::incoming_move::incoming_move()
{
    this->m_vtbl = 0x0087A1FC;
}

combat_inode::incoming_move::incoming_move(const ai::combat_inode::incoming_move &a2) : field_14(a2.field_14)
{
    this->m_vtbl = 0x0087A1FC;
    this->field_4 = a2.field_4;
    this->field_8 = a2.field_8;
    this->field_C = a2.field_C;
    this->field_10 = a2.field_10;
    this->field_90 = a2.field_90;
}

combat_inode::combat_inode() {}

combo_system_move *combat_inode::get_cur_move()
{
    return (combo_system_move *)THISCALL(0x00454410, this);
}

bool combat_inode::needs_hit_react(Float a2)
{
    return (bool)THISCALL(0x004545E0, this, a2);
}

bool combat_inode::has_next_move()
{
    //return this->field_44 != -1;

    return (bool)THISCALL(0x00444F50, this);
}

void combat_inode::left_air()
{
    this->field_68 = 0;
}

}  // namespace ai
