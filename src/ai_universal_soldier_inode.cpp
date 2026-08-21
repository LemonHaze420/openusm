#include "ai_universal_soldier_inode.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"

static constexpr auto NUM_UNI_SOL_COMBAT_SPACING_PTS = 8;

namespace ai {

VALIDATE_SIZE(universal_soldier_inode, 0x78);

universal_soldier_inode::universal_soldier_inode(from_mash_in_place_constructor *a2)
{
    THISCALL(0x006B58F0, this, a2);
}

vector3d universal_soldier_inode::get_combat_spacing_pos(int slot, const vector3d &a3, Float a4)
{
    assert(slot >= 0 && slot < NUM_UNI_SOL_COMBAT_SPACING_PTS);

    static vector3d s_combat_pts[NUM_UNI_SOL_COMBAT_SPACING_PTS]{};
    s_combat_pts[0][0] = 0.0;
    s_combat_pts[0][1] = 0.0;
    s_combat_pts[0][2] = 1.0;

    s_combat_pts[1][0] = 0.70700002;
    s_combat_pts[1][1] = 0.0;
    s_combat_pts[1][2] = 0.70700002;

    s_combat_pts[2][0] = 1.0;
    s_combat_pts[2][1] = 0.0;
    s_combat_pts[2][2] = 0.0;

    s_combat_pts[3][0] = 0.70700002;
    s_combat_pts[3][1] = 0.0;
    s_combat_pts[3][2] = -0.70700002;

    s_combat_pts[4][0] = 0.0;
    s_combat_pts[4][1] = 0.0;
    s_combat_pts[4][2] = -1.0;

    s_combat_pts[5][0] = -0.70700002;
    s_combat_pts[5][1] = 0.0;
    s_combat_pts[5][2] = -0.70700002;

    s_combat_pts[6][0] = -1.0;
    s_combat_pts[6][1] = 0.0;
    s_combat_pts[6][2] = 0.0;

    s_combat_pts[7][0] = -0.70700002;
    s_combat_pts[7][1] = 0.0;
    s_combat_pts[7][2] = 0.70700002;

    auto result = a3 + a4 * s_combat_pts[slot];
    return result;
}

}  // namespace ai
