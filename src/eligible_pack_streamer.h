#pragma once

#include "resource_pack_slot.h"

#include <vector.hpp>

struct eligible_pack_token;
struct eligible_pack_category;
struct ideal_pack_info;
struct eligible_pack;
struct pack_switch_info_t;

struct eligible_pack_streamer {
    bool field_0{false};

private:
    _std::vector<eligible_pack *> eligible_packs;
    _std::vector<eligible_pack_category *> field_14;
    void (*get_ideal_pack_info_callback)(_std::vector<ideal_pack_info> *a1){nullptr};
    _std::vector<pack_switch_info_t> field_28;

public:
    eligible_pack_streamer() = default;

    //0x00547C50
    void init(int a2, int num_streamers, resource_pack_streamer **streamers,
              bool(__cdecl **callbacks)(resource_pack_slot::callback_enum, resource_pack_streamer *,
                                        resource_pack_slot *, limited_timer *),
              void(__cdecl *get_ideal_pack_info)(_std::vector<ideal_pack_info> *));

    //0x00547BA0
    void clear();

    void unlock_pack_slot(resource_pack_slot *slot);

    //0x00547D50
    void fixup_eligible_pack_parent_child_relationships();

    eligible_pack *find_eligible_pack_by_packfile_name_hash(string_hash a2);

    eligible_pack *find_eligible_pack_by_name(string_hash a2);

    //0x00551290
    eligible_pack *add_eligible_pack(const char *a2, const eligible_pack_token &a3, resource_pack_streamer *a4);

    //0x00547D80
    void prioritize();

    void frame_advance(Float a2);

    bool is_idle() const;

    bool is_pack_slot_locked(resource_pack_slot *slot) const;

    eligible_pack *find_eligible_pack_by_token(resource_pack_token &a2) const;

    pack_switch_info_t *get_pack_switch_info(string_hash a2);

    resource_pack_slot *get_eligible_pack_slot(eligible_pack *e_pack);
};

extern void eligible_pack_streamer_patch();
