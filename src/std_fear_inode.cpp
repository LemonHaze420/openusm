#include "std_fear_inode.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(std_fear_inode, 0x7Cu);

std_fear_inode::std_fear_inode() : info_node()
{
    this->field_1C = 0;
    this->field_1D = 0;
    this->field_1E = 0;
    this->field_1F = 0;
    this->field_20 = 0;
    this->field_21 = 0;
    this->field_22 = 0;
    this->field_23 = 0;
    this->field_24 = 0;
    this->field_25 = 0;
    this->field_26 = 0;
    this->field_27 = 0;
    this->field_64 = 0;
    this->field_68 = {0};
    this->field_78 = false;
    this->field_79 = false;
    this->initialize(mash::ALLOCATED);
}

std_fear_inode::std_fear_inode(from_mash_in_place_constructor *a2) : info_node(a2)
{
    this->field_1C = 0;
    this->field_1D = 0;
    this->field_1E = 0;
    this->field_1F = 0;
    this->field_20 = 0;
    this->field_21 = 0;
    this->field_22 = 0;
    this->field_23 = 0;
    this->field_24 = 0;
    this->field_25 = 0;
    this->field_26 = 0;
    this->field_27 = 0;
    this->field_68 = {0};
    this->initialize(mash::ALLOCATED);
}

void std_fear_inode::initialize(mash::allocation_scope a2)
{
#if 0
    ai::std_fear_inode::set_cowering_forced(this, 0);
    ai::std_fear_inode::set_cowering_stop_forced(this, 0);
    ai::std_fear_inode::set_cowering_stop_disabled(this, 0);
    ai::std_fear_inode::set_fleeing_forced(this, 0);
    ai::std_fear_inode::set_fleeing_stop_forced(this, 0);
    ai::std_fear_inode::set_flee_to_location(this, 0);
    ai::std_fear_inode::set_entity_to_flee_from_id(this, Source);
    this->field_68.field_0 = 0;
    this->field_6C = 0.0;
    this->field_74 = 0.0;
    this->field_78 = 0;
    this->field_79 = 0;
    this->field_64 = 0;
    ai::std_fear_inode::add_to_list(this, &ai::std_fear_inode::all_fear_inodes);
#else
    THISCALL(0x006A5090, this, a2);
#endif
}

void std_fear_inode::add_to_list(_std::vector<ai::std_fear_inode *> **a2)
{
    THISCALL(0x006A1810, this, a2);
}

void std_fear_inode::set_cowering_enabled(bool a2)
{
    static const string_hash cowering_enabled_hash{int(to_hash("cowering_enabled"))};

    this->my_param_block.set_pb_int(cowering_enabled_hash, a2, true);
    if (a2 != this->field_1D) {
        this->field_1D = a2;
    }
}

}  // namespace ai
