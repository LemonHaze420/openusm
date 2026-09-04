#pragma once

#include <vector.hpp>

namespace ai {

enum eUniversalSoldierAbility {};

struct universal_soldier_ability_client;

struct universal_soldier_ability_server {
    int m_vtbl;
    eUniversalSoldierAbility field_4;
    int field_8;
    int field_C;
    int field_10;
    int field_14;

    _std::vector<universal_soldier_ability_client *> field_18;

    static void populate_server_array(universal_soldier_ability_server **ability_servers);
};

}  // namespace ai
