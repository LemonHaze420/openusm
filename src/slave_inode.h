#pragma once

#include "entity_base_vhandle.h"
#include "info_node.h"
#include "mcontainer_base.h"

namespace ai {


struct slave_inode : info_node {
    struct master_record {
        entity_base_vhandle actor_handle;
        int als_layer;

        ~master_record() {}
    };

    mContainer_base records;
    master_record *records_data;
    int records_capacity;

    slave_inode();
    explicit slave_inode(from_mash_in_place_constructor *constructor);
    ~slave_inode();
    static void *native_vtable();
    void _unmash(mash_info_struct *info, void *owner);
    void _deactivate();
    void destruct_mashed_class();
    static inline const string_hash default_id{to_hash("slave")};

private:
    void clear_records();
};

}
