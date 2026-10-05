#pragma once

#include "func_wrapper.h"

struct string_hash;

namespace ai {
namespace team {
enum team_enum {};

namespace manager {
team_enum get_team_enum_by_hash(const string_hash &a1);
bool is_enemy(team_enum observer, team_enum candidate);
bool is_friend(team_enum observer, team_enum candidate);
bool is_neutral(team_enum observer, team_enum candidate);
}
}  // namespace team
}  // namespace ai
