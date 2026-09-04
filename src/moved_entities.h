#pragma once

#include "entity.h"
#include "entity_base_vhandle.h"
#include "variable.h"

struct intraframe_trajectory_t;

namespace moved_entities {

struct trajectory_filter_t {
    std::intptr_t m_vtbl;
};

//0x005125D0
void reset_all_moved();

//0x00533D00
void add_moved(vhandle_type<entity> a1);

intraframe_trajectory_t *get_all_trajectories(Float a1, const trajectory_filter_t &filter);

inline auto &moved_count = var<int>(0x0095C890);

inline constexpr auto MAX_MOVED = 600;

inline auto &moved_list = var<vhandle_type<entity>[MAX_MOVED]>(0x0095EFC8);
}  // namespace moved_entities

extern void moved_entities_patch();
