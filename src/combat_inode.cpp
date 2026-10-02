#include "combat_inode.h"

#include "ai_std_combat_target.h"
#include "als_inode.h"
#include "base_ai_core.h"
#include "common.h"
#include "controller_inode.h"
#include "damage_inode.h"

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

void combat_inode::_activate(ai_core *a2)
{
    auto v2 = 100.0f;
    this->field_54 = 0.0;
    this->field_34 = nullptr;
    auto v4 = v2 < this->field_5C;
    this->field_60 = 100.0f;
    if (v4) {
        this->field_60 = this->field_5C;
        this->field_5C = 100.0f;
    }

    if (this->field_58 > this->field_60) {
        this->field_58 = this->field_60;
    }

    if (this->field_58 < this->field_5C) {
        this->field_58 = this->field_5C;
    }

    auto v5 = this->field_60;
    this->field_5C = 0.0f;
    if (v5 < 0.0f) {
        auto v6 = this->field_60;
        this->field_60 = 0.0f;
        this->field_5C = v6;
    }

    if (this->field_58 > (double)this->field_60) {
        this->field_58 = this->field_60;
    }

    if (this->field_58 < (double)this->field_5C) {
        this->field_58 = this->field_5C;
    }

    float v7 = 0.0f;
    this->field_58 = 0.0f;
    if (v7 > this->field_60) {
        this->field_58 = this->field_60;
    }

    if (this->field_58 < this->field_5C) {
        this->field_58 = this->field_5C;
    }

    auto *v8 = a2;
    info_node::_activate(a2);
    this->clear_cur_move();
    this->clear_next_move();
    this->field_6C = 0.0;
    this->field_78 = 0;
    this->field_7C = 0;
    this->field_B4 = 0;
    this->field_82 = 0;
    this->field_83 = 0;
    this->field_88 = -1;
    this->field_8C = -1;
    this->field_85 = 0;
    this->field_86 = 0;

    int v15 = 0;
    auto v9 = v8->field_50.get_optional_pb_int(combat_inode::reject_all_hash, v15, nullptr);
    this->field_81 = v9 != 0;

    v15 = 0;
    auto v10 = v8->field_50.get_optional_pb_int(combat_inode::always_keep_target_hash, v15, nullptr);
    this->field_80 = v10 != 0;

    this->field_24 = (controller_inode *)v8->get_info_node(controller_inode::default_id, false);

    this->field_28 = (als_inode *)v8->get_info_node(als_inode::default_id, true);

    this->field_2C = (combat_target_inode *)v8->get_info_node(combat_target_inode::default_id, false);

    this->field_30 = (damage_inode *)v8->get_info_node(damage_inode::default_id, true);
}

bool combat_inode::needs_hit_react(Float a2)
{
    return (bool)THISCALL(0x004545E0, this, a2);
}

void combat_inode::_clear_cur_move()
{
    this->field_40 = -1;
    this->field_1C = 0;
    this->field_B4 = 0;
    if (this->field_C->is_hero() && !this->has_next_move()) {
        this->field_54 = 0;
    }
}

void combat_inode::clear_cur_move()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xAC));
    func(this);
}

bool combat_inode::has_next_move()
{
    //return this->field_44 != -1;

    return (bool)THISCALL(0x00444F50, this);
}

void combat_inode::_clear_next_move()
{
    this->field_7C = 3;
    this->field_44 = -1;
    this->field_20 = 0;
}

void combat_inode::clear_next_move()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xBC));
    func(this);
}

void combat_inode::left_air()
{
    this->field_68 = 0;
}

}  // namespace ai
