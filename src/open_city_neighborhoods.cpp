#include "open_city_neighborhoods.h"

#include "variable.h"

static Var<const char *[5]> neighborhood_names{0x00937010};

const char *get_neighborhood_name(neighborhood_e neighborhood)
{
    assert((int)neighborhood < sizeof(neighborhood_names()) / sizeof(neighborhood_names()[0]));
    return neighborhood_names()[neighborhood];
}
