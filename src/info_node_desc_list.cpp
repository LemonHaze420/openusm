#include "info_node_desc_list.h"

#include "func_wrapper.h"
#include "utility.h"

namespace ai {

info_node_desc_list::info_node_desc_list() {}

void info_node_desc_list::add_entry(info_node_descriptor entry)
{
    if constexpr (STANDALONE_SYSTEM) {

        for (const auto &existing : field_0) {
            if (existing.field_0.source_hash_code == entry.field_0.source_hash_code &&
                existing.field_4 == entry.field_4)
                return;
        }
        field_0.push_back(entry);
    } else {
        THISCALL(0x006D6E20, this, entry);
    }
}

}  // namespace ai

void info_node_desc_list_patch()
{
    FUNC_ADDRESS(address, &ai::info_node_desc_list::add_entry);
    SET_JUMP(0x006D6E20, address);
}
