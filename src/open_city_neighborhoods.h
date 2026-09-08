#pragma once

enum neighborhood_e {
    NEIGHBORHOOD_QUEENS,
    NEIGHBORHOOD_UPTOWN,
    NEIGHBORHOOD_MIDTOWN,
    NEIGHBORHOOD_DOWNTOWN,
    NEIGHBORHOOD_NONE,
};

extern const char *get_neighborhood_name(neighborhood_e neighborhood);

// 0x005E1000
neighborhood_e get_neighborhood_for_district(int district_id);
