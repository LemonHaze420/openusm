#include "patrol_def.h"

#include "common.h"

VALIDATE_SIZE(patrol_def, 0x28u);

enum {
    PATROL_SKILL_FILTER_EASY = 0,
    PATROL_SKILL_FILTER_MEDIUM = 1,
    PATROL_SKILL_FILTER_HARD = 2,
};

patrol_def::patrol_def(from_mash_in_place_constructor *a2) : field_0(a2)
{
    this->initialize(mash::FROM_MASH);
}

void patrol_def::initialize(mash::allocation_scope a2)
{
    if ( a2 )
    {
        assert(neighborhood_id >= 0);
        
        assert(skill_filter == PATROL_SKILL_FILTER_EASY || skill_filter == PATROL_SKILL_FILTER_MEDIUM || skill_filter == PATROL_SKILL_FILTER_HARD);

        assert(num_nodes >= 0 && num_nodes < 10000);

        assert(script_difficulty >= 0 && script_difficulty < 10000);
    }
    else
    {
        this->clear();
    }
}

void patrol_def::clear()
{
    this->field_4 = vector3d {0.0, 0.0, 0.0};
    this->neighborhood_id = 0;
    this->skill_filter = 2;
    this->num_nodes = 5;
    this->script_difficulty = 0;
    this->field_14 = 5;
    this->field_24 = 0;
}
