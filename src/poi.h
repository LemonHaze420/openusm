#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "vector3d.h"

struct point_of_interest {
    vector3d field_0;
    int field_C;
    float field_10;
    int field_14;
    float field_18;
    vhandle_type<entity> field_1C;

    //0x006C4770
    vector3d get_location() const;
};

namespace poi_manager {


int remove_point_of_interest(int index);


extern void cleanup();

extern void check_init();

extern bool near_violence_poi(const vector3d &a1);

int add_point_of_interest(const vector3d &position, int type, float radius, float duration,
                          vhandle_type<entity> owner);

extern point_of_interest **&poi_list;

}  // namespace poi_manager

extern int &dword_938004;
