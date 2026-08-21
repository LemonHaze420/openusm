#include "ai_universal_soldier_ability.h"

#include "common.h"
#include "func_wrapper.h"

namespace ai {

VALIDATE_SIZE(universal_soldier_ability_server, 0x28u);

void universal_soldier_ability_server::populate_server_array(universal_soldier_ability_server **ability_servers)
{
    CDECL_CALL(0x006A0CA0, ability_servers);
}

}  // namespace ai
