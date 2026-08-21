#include "avltree.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "region_lookup_entry.h"
#include "slab_allocator.h"

VALIDATE_SIZE(TreeNode<region_lookup_entry>, 0x14);

//0x00569E60
template <>
int AvlTree<region_lookup_entry>::addHelper(TreeNode<region_lookup_entry> *a2, TreeNode<region_lookup_entry> *&a3,
                                            TreeNode<region_lookup_entry> *a4)
{
    return THISCALL(0x00569E60, this, a2, &a3, a4);
}

//0x00566EA0
template <>
void AvlTree<region_lookup_entry>::dump(TreeNode<region_lookup_entry> *&a2, int a3)
{
    if (a2 != nullptr) {
        this->dump(a2->field_0, a3);
        this->dump(a2->field_4, a3);
        if (a3 != 0) {
            auto *v4 = a2->m_key;
            if (v4 != nullptr) {
                mem_dealloc(v4, sizeof(*v4));
            }
        }

        auto *v5 = a2;
        if (a2 != nullptr) {
            v5->field_8 = nullptr;
            v5->field_4 = nullptr;
            v5->field_0 = nullptr;
            v5->m_key = nullptr;
            v5->field_10 = 0;

            mem_dealloc(v5, sizeof(*v5));
        }

        a2 = nullptr;
        --this->m_size;
    }
}
