#pragma once

#include "float.hpp"

#include "resource_pack_slot.h"

#include <list.hpp>
#include <vector.hpp>

struct resource_pack_streamer;
struct eligible_pack_streamer;
struct eligible_pack;

struct eligible_pack_category {
    resource_pack_streamer *my_resource_pack_streamer;
    eligible_pack_streamer *my_eligible_pack_streamer;
    bool (*field_8)(resource_pack_slot::callback_enum, resource_pack_streamer *, resource_pack_slot *, limited_timer *);

    _std::vector<eligible_pack *> field_C;
    _std::list<resource_pack_slot *> field_1C;

    //0x00543100
    eligible_pack_category(eligible_pack_streamer *a2, resource_pack_streamer *a3,
                           bool(__cdecl *a4)(resource_pack_slot::callback_enum, resource_pack_streamer *,
                                             resource_pack_slot *, limited_timer *));

    //0x0053E640
    ~eligible_pack_category();

    void clear();

    resource_pack_streamer *get_streamer()
    {
        return my_resource_pack_streamer;
    }

    bool is_pack_slot_locked(resource_pack_slot *a2) const;

    void lock_pack_slot(resource_pack_slot *s);

    void unlock_pack_slot(resource_pack_slot *s);

    //0x005513B0
    void frame_advance(Float a2);

    int find_empty_pack_slot() const;

    int find_lowest_priority_unloadable_loaded_pack_slot();

    int find_lowest_priority_unloadable_loading_pack_slot();
};
