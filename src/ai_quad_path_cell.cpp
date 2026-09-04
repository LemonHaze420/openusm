#include "ai_quad_path_cell.h"

#include "common.h"
#include "func_wrapper.h"

VALIDATE_SIZE(ai_quad_path_cell, 0x50);

ai_quad_path_cell::ai_quad_path_cell() {}

vector3d ai_quad_path_cell::get_edge_midpoint(ai_quad_path_cell *a3)
{
    vector3d result;
    THISCALL(0x004648C0, this, &result, a3);

    return result;
}

void ai_quad_path_cell::un_mash(void *a2, int a3, int a4)
{
    THISCALL(0x00452C60, this, a2, a3, a4);
}
