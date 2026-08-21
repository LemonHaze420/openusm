#pragma once

#include "resource_location.h"

struct resource_pack_location {
    resource_location loc;

    int field_10;
    int field_14;
    int field_18;
    int field_1C;
    int prerequisite_offset;
    int prerequisite_count;
    int field_28;
    int field_2C;

public:
    char m_name[32];

    //0x0051FE80
    resource_pack_location();

    void clear();

    //0x00562D80
    resource_pack_location &operator=(const resource_pack_location &a2);
};
