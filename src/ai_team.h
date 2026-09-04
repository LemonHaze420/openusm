#pragma once

#include "func_wrapper.h"

struct string_hash;

namespace ai {
namespace team {
enum team_enum {};

namespace manager {
team_enum get_team_enum_by_hash(const string_hash &a1);
}
}  // namespace team
}  // namespace ai
