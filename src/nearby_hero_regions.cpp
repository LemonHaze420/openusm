#include "nearby_hero_regions.h"

#include "actor.h"
#include "region.h"
#include "terrain.h"
#include "wds.h"

namespace nearby_hero_regions {

Var<fixed_vector<region *, 15>> regs{0x0095D420};

void update()
{
    auto &nearby = regs();
    nearby.m_size = 0;

    if (g_world_ptr == nullptr || g_world_ptr->the_terrain == nullptr) {
        return;
    }

    auto *hero = g_world_ptr->get_hero_ptr(0);
    if (hero == nullptr) {
        return;
    }

    auto *reg = g_world_ptr->the_terrain->find_region(hero->get_abs_position(), nullptr);
    if (reg != nullptr) {
        nearby.push_back(reg);
    }
}
}  // namespace nearby_hero_regions
