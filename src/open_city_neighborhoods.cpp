#include "open_city_neighborhoods.h"
#include "config.h"

#include "variable.h"

#if STANDALONE_SYSTEM
static constexpr const char *neighborhood_names[] = {
    "Queens",
    "Uptown",
    "Midtown",
    "Downtown",
    "No name!",
};
#else
static Var<const char *[5]> neighborhood_names{0x00937010};
#endif

const char *get_neighborhood_name(neighborhood_e neighborhood)
{
    assert(static_cast<unsigned int>(neighborhood) < 5);
#if STANDALONE_SYSTEM
    return neighborhood_names[neighborhood];
#else
    return neighborhood_names()[neighborhood];
#endif
}

neighborhood_e get_neighborhood_for_district(int district_id)
{
    const auto in_range = [district_id](int first, int last) {
        return district_id >= first && district_id <= last;
    };

    if (in_range(73, 76) || in_range(88, 92) ||
        in_range(103, 108) || in_range(119, 123) ||
        in_range(134, 138)) {
        return NEIGHBORHOOD_QUEENS;
    }
    if (in_range(20, 24) || in_range(36, 40) ||
        in_range(51, 56) || in_range(67, 71) ||
        in_range(83, 87) || in_range(98, 102) ||
        in_range(114, 118) || in_range(128, 133)) {
        return NEIGHBORHOOD_UPTOWN;
    }
    if (in_range(143, 149) || in_range(159, 164) ||
        in_range(175, 180) || in_range(191, 196)) {
        return NEIGHBORHOOD_MIDTOWN;
    }
    if (in_range(207, 212) || in_range(223, 228) ||
        in_range(239, 243) || in_range(254, 257) ||
        in_range(270, 272)) {
        return NEIGHBORHOOD_DOWNTOWN;
    }
    return NEIGHBORHOOD_NONE;
}
