#pragma once

#include "fixedstring.h"

struct level_descriptor_t {
    fixedstring<8> field_0;
    fixedstring<16> field_20;
    fixedstring<4> field_60;
    int m_index_memory_map;
    int field_74;
    int field_78;
    int field_7C;
    int field_80;
    int field_84;
    int field_88;
    int field_8C;
};
