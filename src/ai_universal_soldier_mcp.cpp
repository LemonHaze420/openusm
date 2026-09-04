#include "ai_universal_soldier_mcp.h"

#include "ai_universal_soldier_ability.h"
#include "common.h"
#include "memory.h"
#include "wds.h"

namespace ai {

VALIDATE_SIZE(universal_soldier_MCP, 0x70u);

universal_soldier_MCP::universal_soldier_MCP()
{
    this->field_0 = (g_world_ptr != nullptr ? g_world_ptr->field_158.field_C - 1 : -1);

    universal_soldier_ability_server::populate_server_array(&this->field_14);
    if (global_tokens() == nullptr) {
        auto *mem = mem_alloc(sizeof(12u));
        global_tokens() = new (mem) _std::list<universal_soldier_attack_token *>{};
    }

    for (int i = 0; i < 8; ++i) {
        this->field_30[i].field_0 = {};
        this->field_30[i].field_4 = 0.0;
    }
}

}  // namespace ai
