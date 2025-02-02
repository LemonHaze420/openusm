#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "vector3d.h"

struct point_of_interest {

    vector3d field_0;
    int field_C;
    int field_10;
    int field_14;
    int field_18;
    vhandle_type<entity> field_1C;

    //0x006C4770
    vector3d get_location() const;
};

namespace poi_manager
{

extern void cleanup();

extern void check_init();

extern bool near_violence_poi(const vector3d &a1);

extern point_of_interest **& poi_list;

}

extern int & dword_938004;
