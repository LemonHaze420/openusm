#pragma once

#include "msimpletemplates.h"

struct ai_tentacle_info;

struct ai_tentacle_engine {
    int m_vtbl;
    simple_list<ai_tentacle_engine *>::vars_t simple_list_vars;
    int field_10;
    ai_tentacle_info *field_14;
    int field_18;

    ai_tentacle_engine(ai_tentacle_info *a2);

    static int & id_counter;
};
