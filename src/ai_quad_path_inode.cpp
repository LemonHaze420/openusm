#include "ai_quad_path_inode.h"

#include "ai_path.h"
#include "common.h"
#include "native_info_node_table.h"
#include "slab_allocator.h"
#include <new>

VALIDATE_SIZE(ai::quad_path_inode, 0x20);
VALIDATE_OFFSET(ai::quad_path_inode, path, 0x1C);

namespace ai {
namespace {
void __fastcall quad_path_destruct(quad_path_inode *self, void *)
{
    self->_destruct_mashed_class();
}
}  // namespace

void *quad_path_inode::native_vtable()
{
    static auto table = [] {
        native_inode::table<quad_path_inode, 341> result;
        result[0] = reinterpret_cast<void *>(&quad_path_destruct);
        return result;
    }();
    return table.data();
}

quad_path_inode::quad_path_inode() : path(nullptr)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[341]);
}

quad_path_inode::quad_path_inode(from_mash_in_place_constructor *tag) : info_node(tag), path(nullptr)
{
    m_vtbl = reinterpret_cast<std::intptr_t>(mash_virtual_base::vtable()[341]);

    void *storage = sizeof(ai_path) > slab_allocator::get_max_object_size()
                        ? ::operator new(sizeof(ai_path))
                        : slab_allocator::allocate(sizeof(ai_path), nullptr);
    if (storage != nullptr)
        path = new (storage) ai_path;
}

void quad_path_inode::_destruct_mashed_class()
{
    if (path != nullptr) {
        path->~ai_path();
        if (sizeof(ai_path) > slab_allocator::get_max_object_size())
            ::operator delete(path);
        else
            slab_allocator::deallocate(path, nullptr);
    }
    path = nullptr;
    info_node::_destruct_mashed_class();
}
}  // namespace ai
