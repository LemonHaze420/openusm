#include "region_lookup_cache.h"


#include "region.h"
#include "terrain.h"


#include "wds.h"

void region_lookup_cache::init(terrain *a1)
{
    total_regions() = a1->total_regions;
    region_ptrs() = static_cast<region **>(::operator new(sizeof(region *) * total_regions()));
    district_ids() = static_cast<int *>(::operator new(sizeof(int) * total_regions()));
    for (int i = 0; i < total_regions(); ++i) {
        region_ptrs()[i] = a1->regions[i];
        district_ids()[i] = a1->regions[i]->district_id;
    }
    initialized() = true;
}

region *region_lookup_cache::lookup_by_district_id(int a1)
{
    if (!initialized())
        init(g_world_ptr->the_terrain);
    int index = 0;
    while (index < total_regions() && district_ids()[index] != a1)
        ++index;
    if (index == total_regions())
        return nullptr;
    if (index <= 10)
        return region_ptrs()[index];
    auto *displaced_region = region_ptrs()[10];
    const int displaced_district = district_ids()[10];
    for (int i = 10; i > 0; --i) {
        region_ptrs()[i] = region_ptrs()[i - 1];
        district_ids()[i] = district_ids()[i - 1];
    }
    region_ptrs()[0] = region_ptrs()[index];
    district_ids()[0] = district_ids()[index];
    region_ptrs()[index] = displaced_region;
    district_ids()[index] = displaced_district;
    return region_ptrs()[0];
}

void region_lookup_cache::term()
{
    if (region_lookup_cache::initialized()) {
        operator delete[](region_lookup_cache::region_ptrs());
        operator delete[](region_lookup_cache::district_ids());
        region_lookup_cache::initialized() = false;
    }
}
