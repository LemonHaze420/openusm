#include "ai_universal_soldier_inode.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "base_ai_core.h"
#include "ai_voice_box_inode.h"
#include "ai_universal_soldier_mcp.h"

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

bool universal_soldier_inode::say_gab(string_hash sound, int interruption, int priority)
{
    if (field_74 > 0.0f)
        return false;
    auto *voice = static_cast<voice_box_inode *>(field_8->get_info_node(voice_box_inode::default_id, false));
    if (voice == nullptr || !voice->can_gab())
        return false;
    field_74 = 5.0f;
    return voice->say_gab(sound, interruption, priority, nullptr);
}
namespace {
auto *token_list(universal_soldier_inode *owner)
{
    return owner->field_30 ? owner->field_30->field_24 : universal_soldier_MCP::global_tokens();
}
}

void universal_soldier_inode::release_attack_token(universal_soldier_inode *owner)
{
    if (owner == nullptr || owner->field_20 == nullptr)
        return;
    auto *token = owner->field_20;
    auto *tokens = token_list(token->owner);
    for (auto it = tokens->begin(); it != tokens->end(); ++it) {
        if (*it == token) {
            tokens->erase(it);
            break;
        }
    }
    if (token->owner->field_20 == token)
        token->owner->field_20 = nullptr;
    delete token;
    owner->field_20 = nullptr;
}

void universal_soldier_inode::redeem_attack_token(universal_soldier_inode *owner)
{
    if (owner == nullptr || owner->field_20 == nullptr || owner->field_20->redeemed)
        return;
    auto *token = owner->field_20;
    token->redeemed = true;
    token->remaining = owner->field_50;
    auto *tokens = token_list(owner);
    for (auto it = tokens->begin(); it != tokens->end();) {
        auto *other = *it;
        if (!other->redeemed && other->target == token->target) {
            it = tokens->erase(it);
            if (other->owner->field_20 == other)
                other->owner->field_20 = nullptr;
            delete other;
        } else {
            ++it;
        }
    }
}

}  // namespace ai
