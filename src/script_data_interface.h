#pragma once

#include "conglomerate_interface.h"

#include "mstring.h"
#include "vector3d.h"

struct script_data_interface : conglomerate_interface
{
    char field_C[0x28u];

    mString field_34;
    mString field_44;
    mString field_54;
    mString field_64;
    vector3d field_74;
    vector3d field_80;

    void _un_mash(generic_mash_header *a2, void *a3, void *a4, generic_mash_data_ptrs *a5);

    //virtual
    void release_ifc();
};

extern void script_data_interface_patch();
