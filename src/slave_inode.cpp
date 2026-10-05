#include "slave_inode.h"

#include "base_ai_core.h"
#include "common.h"
#include "mash_config.h"
#include "mash_info_struct.h"
#include "memory.h"

#include <algorithm>
#include <array>

namespace ai {

VALIDATE_SIZE(slave_inode, 0x2C);
VALIDATE_SIZE(slave_inode::master_record, 8);


#if STANDALONE_SYSTEM
namespace {
void __fastcall slave_mashed_destruct(slave_inode *node) { node->destruct_mashed_class(); }
void __fastcall slave_unmash(slave_inode *node, void *, mash_info_struct *info, void *owner)
{
    node->_unmash(info, owner);
}
slave_inode *__fastcall slave_delete(slave_inode *node, void *, unsigned char flags)
{
    node->~slave_inode();
    if ((flags & 1) != 0)
        mem_dealloc(node, sizeof(*node));
    return node;
}
int __fastcall slave_type(const slave_inode *) { return 408; }
bool __fastcall slave_subclass(const slave_inode *, void *, int type) { return type == 537 || type == 573; }
void __fastcall slave_deactivate(slave_inode *node) { node->_deactivate(); }
int __fastcall slave_size(const slave_inode *) { return sizeof(slave_inode); }
}

void *slave_inode::native_vtable()
{
    static std::array<void *, 12> table = [] {
        std::array<void *, 12> result;
        std::copy_n(static_cast<void **>(info_node::native_vtable()), result.size(), result.begin());
        result[0x00 / 4] = reinterpret_cast<void *>(&slave_mashed_destruct);
        result[0x04 / 4] = reinterpret_cast<void *>(&slave_unmash);
        result[0x08 / 4] = reinterpret_cast<void *>(&slave_delete);
        result[0x0C / 4] = reinterpret_cast<void *>(&slave_type);
        result[0x10 / 4] = reinterpret_cast<void *>(&slave_subclass);
        result[0x24 / 4] = reinterpret_cast<void *>(&slave_deactivate);
        result[0x2C / 4] = reinterpret_cast<void *>(&slave_size);
        return result;
    }();
    return table.data();
}
#else
void *slave_inode::native_vtable() { return reinterpret_cast<void *>(0x0087DD04); }
#endif

slave_inode::slave_inode()
    : info_node(), records(), records_data(nullptr), records_capacity(0)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[408]);
#else
    m_vtbl = 0x0087DD04;
#endif
}

slave_inode::slave_inode(from_mash_in_place_constructor *constructor)
    : info_node(constructor), records(constructor)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[408]);
#else
    m_vtbl = 0x0087DD04;
#endif
}

slave_inode::~slave_inode()
{

    clear_records();
}

void slave_inode::_unmash(mash_info_struct *info, void *owner)
{
    info_node::_unmash(info, owner);
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    records.m_size = *reinterpret_cast<int *>(info->read_from_buffer(mash::SHARED_BUFFER, 4, 4));
#endif
    if (records_data)
        records_data = reinterpret_cast<master_record *>(info->read_from_buffer(sizeof(master_record) * records.m_size, 4));
    records.field_0 = static_cast<int>(info->mash_image_ptr[0] + info->buffer_size_used[0] -
                                      reinterpret_cast<unsigned char *>(&records));
}

void slave_inode::clear_records()
{
    if (!records.is_pointer_in_mash_image(records_data))
        delete[] records_data;
    records_data = nullptr;
    records_capacity = 0;
    records.clear();
}

void slave_inode::_deactivate()
{
    if (records.m_size != 0) {
        clear_records();
        field_8->pop_base_machine(3);
    }
}

void slave_inode::destruct_mashed_class()
{
    clear_records();
    records.destruct_mashed_class();
    info_node::_destruct_mashed_class();
}

}
