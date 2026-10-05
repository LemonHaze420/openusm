#pragma once

#include <cstdint>

struct game_save_timestamp {
    std::int16_t year;
    std::int16_t month;
    std::int16_t day;
    std::int16_t hour;
    std::int16_t minute;
    std::int16_t second;
};

struct game_data_essentials {
    game_save_timestamp timestamp;
    int field_C;
    int field_10;
    char field_14[25];
    char field_2E[12];

    //0x00580DD0
    game_data_essentials();
};
