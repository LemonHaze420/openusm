#pragma once

#include "info_node.h"
#include "vector3d.h"

struct entity;

namespace ai {
struct damage_inode : info_node {
    int field_1C;
    float field_20;

    damage_inode();
    explicit damage_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_core *core);


    void apply_damage(actor *source, int amount, vector3d direction, const string_hash &attack,
                      const string_hash &category, const string_hash &reaction, bool force_reaction, int combo_type,
                      bool skip_combat);

    void apply_forced_damage(int amount, vector3d direction, const string_hash &attack, bool force_reaction);

    void apply_subdue(entity *source, float amount, vector3d direction);
    void set_subdue(entity *source, float amount, vector3d direction);


    void _frame_advance(Float) {}
    int _get_virtual_type_enum() const
    {
        return 343;
    }
    int _get_mash_sizeof() const
    {
        return sizeof(*this);
    }
    static inline string_hash default_id{int(to_hash("damage_inode"))};
};
}  // namespace ai
