#pragma once

#include "actor.h"
#include "entity_base_vhandle.h"

#include <vector.hpp>

namespace ai {

struct universal_soldier_ability_server;
struct universal_soldier_attack_token;
struct universal_soldier_inode;

struct universal_soldier_MCP {
    int field_0;
    _std::vector<universal_soldier_inode *> all_soldiers;
    universal_soldier_ability_server *field_14;
    int field_18;
    int field_1C;
    int field_20;
    int field_24;
    int field_28;
    int field_2C;
    struct {
        vhandle_type<actor> field_0;
        float field_4;
    } field_30[8];

    universal_soldier_MCP();

    static inline Var<_std::list<universal_soldier_attack_token *> *> global_tokens{0x0096BE64};
};

}  // namespace ai
