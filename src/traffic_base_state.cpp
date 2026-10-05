#include "traffic_base_state.h"

#include "als_inode.h"
#include "common.h"
#include "info_node_desc_list.h"
#include "traffic_inode.h"

#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(traffic_base_state, 0x30);

namespace {
void *__fastcall traffic_finalize(traffic_base_state *self, void *, unsigned flags)
{

    self->~traffic_base_state();
    if (flags & 1)
        mash_virtual_base::operator delete(self, sizeof(traffic_base_state));
    return self;
}
unsigned __fastcall traffic_type(const traffic_base_state *, void *) { return 421; }
bool __fastcall traffic_subclass(const traffic_base_state *, void *, mash::virtual_types_enum type)
{

    return type == 535 || type == 567 || type == 573;
}
void __fastcall traffic_nodes(traffic_base_state *self, void *, info_node_desc_list &nodes)
{
    self->get_info_node_list(nodes);
}
int __fastcall traffic_size(const traffic_base_state *, void *) { return sizeof(traffic_base_state); }
}

void *traffic_base_state::native_vtable()
{
    static auto table = [] {
        std::array<void *, 16> result;
        std::copy_n(static_cast<void **>(enhanced_state::native_vtable()), result.size(), result.data());



        result[2] = bit_cast<void *>(&traffic_finalize);
        result[3] = bit_cast<void *>(&traffic_type);
        result[4] = bit_cast<void *>(&traffic_subclass);
        result[9] = bit_cast<void *>(&traffic_nodes);
        result[13] = bit_cast<void *>(&traffic_size);
        return result;
    }();
    return table.data();
}

traffic_base_state::traffic_base_state()
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x8778F8;
}

traffic_base_state::traffic_base_state(from_mash_in_place_constructor *constructor)
    : enhanced_state(constructor)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x8778F8;
}

void traffic_base_state::get_info_node_list(info_node_desc_list &nodes)
{
    nodes.add_entry({als_inode::default_id, 333});
    nodes.add_entry({traffic_inode::default_id, 422});
}

}
