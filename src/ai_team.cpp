#include "ai_team.h"

#include "string_hash.h"
#include "common.h"
#include "variable.h"

#include <array>

namespace {
constexpr std::array<unsigned int, 15> team_hashes{
    to_hash("SPIDERMAN"), to_hash("VENOM"), to_hash("BOSS"), to_hash("NONATTACK_BOSS"),
    to_hash("SHIELD"), to_hash("POLICE"), to_hash("GANG_SKULLS"), to_hash("GANG_HELLIONS"),
    to_hash("GANG_FTB"), to_hash("GANG_SKINHEAD"), to_hash("GANG_MERC"), to_hash("GANG_SRK"),
    to_hash("TRASK"), to_hash("CIVILIAN"), to_hash("PEDESTRIAN")
};


constexpr unsigned int friendly = (1u << 0) | (1u << 4) | (1u << 5);
constexpr unsigned int hostile = (1u << 1) | (1u << 2) | (1u << 3) |
    (1u << 6) | (1u << 7) | (1u << 8) | (1u << 9) | (1u << 10) | (1u << 11) | (1u << 12);
constexpr std::array<unsigned int, 16> enemy_masks{
    hostile & ~(1u << 3), 0x7ffdu, (friendly | (1u << 1)) & ~(1u << 5),
    (friendly | (1u << 1)) & ~(1u << 5), hostile, hostile,
    friendly | (1u << 1) | (1u << 7), friendly | (1u << 1) | (1u << 6),
    friendly | (1u << 1) | (1u << 9) | (1u << 10) | (1u << 11),
    friendly | (1u << 1) | (1u << 8) | (1u << 10) | (1u << 11),
    friendly | (1u << 1) | (1u << 8) | (1u << 9) | (1u << 11),
    friendly | (1u << 1) | (1u << 8) | (1u << 9) | (1u << 10),
    friendly, hostile, 0, hostile
};
constexpr std::array<unsigned int, 16> friend_masks{
    friendly, 0, hostile & ~(1u << 1), hostile & ~(1u << 1), friendly, friendly,
    hostile & ~((1u << 1) | (1u << 7)), hostile & ~((1u << 1) | (1u << 6)),
    hostile & ~((1u << 1) | (1u << 9) | (1u << 10) | (1u << 11)),
    hostile & ~((1u << 1) | (1u << 8) | (1u << 10) | (1u << 11)),
    hostile & ~((1u << 1) | (1u << 8) | (1u << 9) | (1u << 11)),
    hostile & ~((1u << 1) | (1u << 8) | (1u << 9) | (1u << 10)),
    hostile, friendly | (1u << 13) | (1u << 14),
    friendly | (1u << 13) | (1u << 14), 0x7fffu
};
}

namespace ai {
namespace team {
namespace manager {
team_enum get_team_enum_by_hash(const string_hash &a1)
{
    for (unsigned int i = 0; i < team_hashes.size(); ++i) {
        if (team_hashes[i] == a1.source_hash_code)
            return static_cast<team_enum>(i);
    }
    return static_cast<team_enum>(15);
}

bool is_enemy(team_enum observer, team_enum candidate)
{
    if constexpr (STANDALONE_SYSTEM) {
        return (enemy_masks[observer] & (1u << candidate)) != 0;
    } else {
        const auto &masks = var<unsigned int[15]>(0x0096C93C);
        const auto &relations = var<unsigned int[30]>(0x0096C8B0);
        return (masks[candidate] & relations[2 * observer]) != 0;
    }
}

bool is_friend(team_enum observer, team_enum candidate)
{
    if constexpr (STANDALONE_SYSTEM) {
        return (friend_masks[observer] & (1u << candidate)) != 0;
    } else {
        const auto &masks = var<unsigned int[15]>(0x0096C93C);
        const auto &relations = var<unsigned int[30]>(0x0096C8B4);
        return (masks[candidate] & relations[2 * observer]) != 0;
    }
}

bool is_neutral(team_enum observer, team_enum candidate)
{
    return !is_enemy(observer, candidate) && !is_friend(observer, candidate);
}
}  // namespace manager
}  // namespace team
}  // namespace ai
