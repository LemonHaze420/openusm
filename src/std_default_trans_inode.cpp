#include "std_default_trans_inode.h"

#include "common.h"

namespace ai {

VALIDATE_SIZE(std_default_trans_inode, 0x34);

std_default_trans_inode::std_default_trans_inode()
{

}

void std_default_trans_inode::set_enabled(bool a2)
{
    static const string_hash enabled_hash {int(to_hash("enabled"))};

    this->my_param_block.set_pb_int(enabled_hash, a2, true);
    if ( a2 != this->field_1C ) {
        this->field_1C = a2;
    }
}

} // namespace ai
