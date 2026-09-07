#pragma once

#include "variable.h"

#include <cstdint>
#include <vector.hpp>

struct entity;
struct item;
struct entity_base;
struct string_hash;

extern void release_generic_mash(void *a1);

//0x004FE6A0
extern void construct_v_table_lookup();

//0x004FF610
extern entity_base *parse_entity_mash(_std::vector<entity *> *ent_vec_ptr, _std::vector<item *> *item_vec_ptr, void *a3,
                                      const string_hash *a7, void *a8, bool a9);

extern void entity_mash_patch();

#if STANDALONE_SYSTEM
extern int ent_v_table_lookup[28];
extern int ent_size_lookup[28];
extern std::array<int, 13> ifc_v_table_lookup;
#else
extern Var<int[28]> ent_v_table_lookup;
extern Var<int[28]> ent_size_lookup;
extern std::array<int, 11> &ifc_v_table_lookup;
#endif

extern uint16_t pc_entity_mash_type(uint16_t type);
extern uint32_t entity_mash_size(uint16_t type);

enum eEntityMashTypeEnum {};

extern void fix_entity_v_table(char *addr, eEntityMashTypeEnum a2);

enum eEntityMashIFCTypeEnum {};

extern void fix_ifc_v_table(char *addr, eEntityMashIFCTypeEnum ifc_type);

static inline constexpr uint8_t MASH_V_TABLE_VAL[4] = {173, 91, 206, 122};
