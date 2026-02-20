#include "patrol_def.h"

#include "common.h"
#include "mash_info_struct.h"

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

void patrol_def::finalize(mash::allocation_scope)
{
    this->clear();
}

void patrol_def::destruct_mashed_class()
{
    this->finalize(mash::FROM_MASH);
    this->field_0.destruct_mashed_class();
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

void patrol_def::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, nullptr);
}
