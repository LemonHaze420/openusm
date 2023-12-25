#pragma once

#include "float.hpp"
#include "variable.h"

#include <list.hpp>

namespace als {

struct animation_logic_system;

struct animation_logic_system_interface 
{
    int m_vtbl;
    static void frame_advance_pre_controller_all_alses(Float a1);

    void force_update();

    //0x00492FC0
    void force_update(Float a2);

    struct value_t {
        animation_logic_system *field_0;
        bool field_4;
    };

    static inline Var<_std::list<value_t>> the_als_list {0x009597C0};
};

}


extern void als_animation_logic_system_interface_patch();
