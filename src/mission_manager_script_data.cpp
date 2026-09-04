#include "mission_manager_script_data.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(mission_manager_script_data, 0xE8);

mission_manager_script_data::mission_manager_script_data()
{
    THISCALL(0x005E9760, this);
}

mission_manager_script_data::~mission_manager_script_data()
{
    THISCALL(0x005E8F70, this);
}

void mission_manager_script_data::copy(const mission_manager_script_data &a2)
{
    THISCALL(0x005E98A0, this, &a2);
}
