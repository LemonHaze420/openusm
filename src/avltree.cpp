#include "avltree.h"

#include "common.h"
#include "memory.h"
#include "region_lookup_entry.h"

#include <new>

VALIDATE_SIZE(TreeNode<region_lookup_entry>, 0x14);

namespace {
using RegionNode = TreeNode<region_lookup_entry>;

int node_height(const RegionNode *node)
{
    return node != nullptr ? static_cast<signed char>(node->field_10) : -1;
}

void update_height(RegionNode *node)
{
    const int left = node_height(node->field_0);
    const int right = node_height(node->field_4);
    node->field_10 = static_cast<char>((left > right ? left : right) + 1);
}

int compare_nodes(const RegionNode *node, const RegionNode *other)
{
    if (node->m_key == nullptr)
        return -1;
    const auto key = node->m_key->field_0.source_hash_code;
    const auto other_key = other->m_key->field_0.source_hash_code;
    return key > other_key ? 1 : -(key < other_key);
}


void rotate_right(RegionNode *&root)
{
    RegionNode *parent = root->field_8;
    RegionNode *left = root->field_0;
    root->field_0 = left->field_4;
    left->field_4 = root;
    update_height(root);
    update_height(left);
    root = left;
    root->field_8 = parent;
    if (root->field_0 != nullptr)
        root->field_0->field_8 = root;
    root->field_4->field_8 = root;
}


void rotate_left(RegionNode *&root)
{
    RegionNode *parent = root->field_8;
    RegionNode *right = root->field_4;
    root->field_4 = right->field_0;
    right->field_0 = root;
    update_height(root);
    update_height(right);
    root = right;
    root->field_8 = parent;
    root->field_0->field_8 = root;
    if (root->field_4 != nullptr)
        root->field_4->field_8 = root;
}
}  // namespace

//0x00569E60
template <>
int AvlTree<region_lookup_entry>::addHelper(RegionNode *node, RegionNode *&root, RegionNode *parent)
{
    if (root == nullptr) {
        node->field_8 = parent;
        root = node;
        ++this->m_size;
        return 0;
    }

    const int comparison = compare_nodes(node, root);
    if (comparison < 0) {
        this->addHelper(node, root->field_0, root);
        if (node_height(root->field_0) - node_height(root->field_4) == 2) {
            if (compare_nodes(node, root->field_0) >= 0)
                rotate_left(root->field_0);
            rotate_right(root);
        } else {
            update_height(root);
        }
    } else if (comparison > 0) {
        this->addHelper(node, root->field_4, root);
        if (node_height(root->field_4) - node_height(root->field_0) == 2) {
            if (compare_nodes(node, root->field_4) <= 0)
                rotate_right(root->field_4);
            rotate_left(root);
        } else {
            update_height(root);
        }
    }

    return comparison;
}


template <>
void AvlTree<region_lookup_entry>::add(region_lookup_entry *key)
{
    if (key == nullptr)
        return;

    void *storage = mem_alloc(sizeof(RegionNode));
    RegionNode *node = storage != nullptr ? new (storage) RegionNode{nullptr, nullptr, nullptr, key, 0} : nullptr;
    this->addHelper(node, this->field_0, nullptr);
}

template <>
TreeNode<region_lookup_entry> *AvlTree<region_lookup_entry>::findHelper(TreeNode<region_lookup_entry> *node,
                                                                        region_lookup_entry *key) const
{
    while (node != nullptr && key != nullptr) {
        auto node_hash = node->m_key->field_0.source_hash_code;
        auto key_hash = key->field_0.source_hash_code;
        if (node_hash == key_hash)
            return node;
        node = key_hash < node_hash ? node->field_0 : node->field_4;
    }
    return nullptr;
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
