#pragma once

#include "script_library_class.h"

#include "vector3d.h"

using vm_vector3d_t = vector3d;

struct vm_stack;

struct slf__angle_between__vector3d__vector3d__t : script_library_class::function {
    slf__angle_between__vector3d__vector3d__t(const char *a3);

    struct parms_t {
        vm_vector3d_t v0;
        vm_vector3d_t v1;
    };

    //0x00672070
    //virtual
    bool operator()(vm_stack &stack, script_library_class::function::entry_t a2);
};
