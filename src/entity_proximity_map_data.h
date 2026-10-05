#pragma once

#include <cstdint>

struct entity;

struct entity_proximity_map_data {
    uint8_t min_x;
    uint8_t min_y;
    uint8_t max_x;
    uint8_t max_y;
    entity *ent;
    entity_proximity_map_data *next;
    int map_level;
};
