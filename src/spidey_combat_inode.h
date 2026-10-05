#pragma once

#include "player_combat_inode.h"

struct entity;
struct spidey_combat_web;

namespace ai {

struct spidey_combat_inode : player_combat_inode {
    float field_330;
    int field_334;
    int field_338;
    int field_33C;
    int field_340;
    int field_344;
    spidey_combat_web *field_348;
    spidey_combat_web *field_34C;

    spidey_combat_inode();
    explicit spidey_combat_inode(from_mash_in_place_constructor *tag);
    ~spidey_combat_inode();
    static void *native_vtable();
    void _destruct_mashed_class();
    void _frame_advance(Float delta);
    void _deactivate();
    void finalize();
    void activate_sense() { field_338 = 2; }
    void activate_web(entity *source, float x, float y, float z);
    void deactivate_web(entity *source);

    //0x00697420
    //virtual
    void _activate(ai_core *a2);

    //0x0069B810
    //virtual
    void update_pending_move(const combat_inode::incoming_move &move);
};
}  // namespace ai

extern void spidey_combat_inode_patch();
