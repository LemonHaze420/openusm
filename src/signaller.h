#pragma once

#include "entity_base.h"

struct signaller : entity_base {
    using base_type = entity_base_vhandle;

    int field_44;

    signaller() = default;

    //0x004F90D0
    signaller(bool a2);

    //0x004F90F0
    signaller(const string_hash &a2, uint32_t a3, bool a4);

    ~signaller();

    bool sub_48AE20();

    int get_entity_size();

    //0x004F9130
    void release_mem();
};
