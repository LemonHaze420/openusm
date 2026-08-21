#include "ai_team.h"

#include "string_hash.h"

namespace ai {
namespace team {
namespace manager {
team_enum get_team_enum_by_hash(const string_hash &a1)
{
    team_enum (*func)(const string_hash *) = CAST(func, 0x006BDB80);
    return func(&a1);
}
}  // namespace manager
}  // namespace team
}  // namespace ai
