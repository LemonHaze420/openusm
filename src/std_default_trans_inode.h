#pragma once

#include "info_node.h"

#include "variable.h"

namespace ai {

struct std_default_trans_inode : info_node {
    int field_1C;
    int field_20;
    bool field_24;
    bool field_25;
    bool field_26;
    int field_28;
    int field_2C;
    int field_30;

    std_default_trans_inode();

    void set_enabled(bool a2);

    static const inline string_hash default_id{static_cast<int>(to_hash("std_default_trans_inode"))};
};

}  // namespace ai
