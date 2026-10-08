#include "retaliation_inode.h"

#include "actor.h"
#include "common.h"
#include "damage_interface.h"
#include "mashed_state.h"
#include "utility.h"
#include "wds.h"
#include "native_info_node_table.h"

namespace ai {

VALIDATE_SIZE(retaliation_inode, 0x34);
VALIDATE_OFFSET(retaliation_inode, last_health, 0x24);
VALIDATE_OFFSET(retaliation_inode, thresholds, 0x2C);

namespace {
void __fastcall retaliation_activate(retaliation_inode *self, void *, ai_core *core)
{
    self->_activate(core);
}
void __fastcall retaliation_reset(retaliation_inode *self, void *)
{
    self->_reset();
}
}

void *retaliation_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<retaliation_inode, 54> result;
        result[8] = reinterpret_cast<void *>(&retaliation_activate);
        result[10] = reinterpret_cast<void *>(&retaliation_reset);
        return result;
    }();
    return table.data();
}


retaliation_inode::retaliation_inode(from_mash_in_place_constructor *tag) : info_node(tag)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[54]);
}


void retaliation_inode::_activate(ai_core *core)
{
    info_node::_activate(core);
    reset();
}


void retaliation_inode::_reset()
{
    for (int index = 0; index < 2; ++index) {
        counters[index] = 0;
        last_health[index] = field_C->damage_ifc()->field_1FC.field_0[0];
        thresholds[index] = 1.0f;
    }
}


bool retaliation_inode::calc_damage_since_last_retaliation(int index) const
{
    const auto *damage = field_C->damage_ifc();
    return (last_health[index] - damage->field_1FC.field_0[0]) / damage->field_1FC.field_0[2] >= thresholds[index];
}


void retaliation_inode::configure(const mashed_state *state)
{
    const auto &params = state->field_0;
    if (params.get_optional_pb_int(string_hash{int(to_hash("reset_retaliation"))}, 0, nullptr))
        reset();
    thresholds[0] =
        params.get_optional_pb_float(string_hash{int(to_hash("retaliate_1_trigger"))}, thresholds[0], nullptr);
    thresholds[1] =
        params.get_optional_pb_float(string_hash{int(to_hash("retaliate_2_trigger"))}, thresholds[1], nullptr);
}


void retaliation_inode::record_retaliation(int index)
{
    for (; index < 2; ++index) {
        counters[index] = bit_cast<int>(g_world_ptr->time_manager.field_8);
        last_health[index] = field_C->damage_ifc()->field_1FC.field_0[0];
    }
}

}
