#pragma once

#include "base_full_target_inode.h"

#include "actor.h"
#include "entity_base_vhandle.h"
#include "mvector.h"
#include "mVectorBasic.h"
#include "string_hash.h"
#include "variable.h"

namespace ai {
struct combat_target_inode : base_full_target_inode {
    bool field_84;

    //0x004406F0
    combat_target_inode();
    explicit combat_target_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    void _activate(ai_core *core);
    bool target_plausible(vhandle_type<actor> candidate);
    int _get_virtual_type_enum() const
    {
        return 351;
    }
    static mVectorBasic<vhandle_type<actor>> &combat_list();

#if STANDALONE_SYSTEM

    static string_hash team_hash()
    {
        return string_hash{int(to_hash("team"))};
    }
#else
    static inline Var<string_hash> team_hash{0x0096C470};
#endif

    static inline string_hash default_id{to_hash("COMBAT_TARGET")};
};

struct venom_combat_target_inode : combat_target_inode {
    float field_88;
    float field_8C;
    float field_90;

    venom_combat_target_inode();

    explicit venom_combat_target_inode(from_mash_in_place_constructor *tag);
    static void *native_vtable();
    vhandle_type<actor> get_player_target();
    vhandle_type<actor> player_style_get_target();
    int _get_virtual_type_enum() const
    {
        return 356;
    }
    int _get_mash_sizeof() const
    {
        return sizeof(*this);
    }
    void _frame_advance(Float delta);
    vector3d get_look_direction() const
    {
        return {field_88, field_8C, field_90};
    }
};

struct player_web_target_inode {
    static void add_to_web_targets_list(vhandle_type<actor> a1);

    static inline auto &web_targets_list = var<mVectorBasic<vhandle_type<actor>>>(0x0096BFD4);
};
}  // namespace ai
