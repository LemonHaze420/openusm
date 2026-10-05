#pragma once

#include "entity_base_vhandle.h"
#include "info_node.h"
#include "mcontainer_base.h"

namespace ai {



struct prop_physics_inode : info_node {
    struct prop_record {
        entity_base_vhandle actor_handle;
        float ballistic_target[3];
        bool target_pending;
        char field_11[3];
        float target_delay;
    };

    mContainer_base records;
    prop_record *records_data;
    int records_capacity;
    bool field_2C;

    prop_physics_inode();
    explicit prop_physics_inode(from_mash_in_place_constructor *constructor);
    ~prop_physics_inode();
    static void *native_vtable();
    void _frame_advance(Float elapsed_seconds);
    void _unmash(mash_info_struct *info, void *owner);
    void _deactivate();
    void destruct_mashed_class();
    static inline const string_hash default_id{"prop_physics"};

private:
    void clear_records();
};

}
