#pragma once

#include "func_wrapper.h"
#include "log.h"
#include "mash.h"
#include "mcontainer_base.h"
#include "memory.h"
#include "trace.h"

#include <type_traits>

struct from_mash_in_place_constructor;
struct mash_info_struct;

template <typename T>
struct mAvlNode {
    using value_type = T;

    T *m_key;
    mAvlNode<T> *m_left;
    mAvlNode<T> *m_right;
    mAvlNode<T> *m_parent;
    int8_t m_height;

    explicit mAvlNode(T *a2)
    {
        this->initialize(mash::ALLOCATED);
        this->m_key = a2;
    }

    ~mAvlNode() = default;

    void initialize(mash::allocation_scope a2)
    {
        if (a2 == mash::ALLOCATED) {
            this->m_key = nullptr;
            this->m_height = 0;
            this->m_parent = nullptr;
            this->m_right = nullptr;
            this->m_left = nullptr;
        }
    }

    static int8_t getHeight(mAvlNode<value_type> *node)
    {
        return (node != nullptr ? node->m_height : -1);
    }

    void updateHeight(mAvlNode<value_type> *left, mAvlNode<value_type> *right)
    {
        this->m_height = std::max(getHeight(left), getHeight(right)) + 1;
    }

    int heightDiff() const
    {
        return getHeight(m_left) - getHeight(m_right);
    }

    void unmash(mash_info_struct *a3, void *a4);

    void destruct_mashed_class()
    {
        if (this->m_key != nullptr) {
            this->m_key->destruct_mashed_class();
            this->m_key = nullptr;
        }
    }
};

template <typename T>
class mAvlTree : mContainer_base {
    using value_type = T;
    using node_type = mAvlNode<T>;

    node_type *m_head;
    bool field_C;

public:
    struct iterator {
        mAvlNode<T> *field_0;

        bool operator!=(const iterator &iter) const
        {
            return this->field_0 != iter.field_0;
        }

        mAvlNode<T> &operator*()
        {
            return (*this->field_0);
        }

        auto &operator++()
        {
            if (this->field_0 != nullptr) {
                this->iterate();
            }

            return (*this);
        }

        //0x00743470
        void iterate()
        {
#if 1
            auto *v1 = this->field_0->m_right;
            if (v1 != nullptr) {
                this->field_0 = v1;
                if (v1->m_left != nullptr) {
                    mAvlNode<value_type> *v2;
                    do {
                        v2 = this->field_0->m_left;
                        this->field_0 = v2;
                    } while (v2->m_left != nullptr);
                }
            } else {
                if (this->field_0->m_parent) {
                    mAvlNode<value_type> *v3;
                    do {
                        v3 = this->field_0->m_parent;
                        if (this->field_0 != v3->m_right) {
                            break;
                        }

                        this->field_0 = v3;
                    } while (v3->m_parent != nullptr);
                }

                this->field_0 = this->field_0->m_parent;
            }
#else
            THISCALL(0x00743470, this);
#endif
        }
    };

    mAvlTree()
    {
        this->initialize(mash::ALLOCATED);
    }

    //mAvlTree(int a1) : mContainer_base(a1) {}

    mAvlTree(from_mash_in_place_constructor *a2) : mContainer_base(a2) {}

    ~mAvlTree()
    {
        this->finalize(mash::ALLOCATED);
    }

    bool from_mash() const
    {
        return this->field_0 != 0;
    }

    void initialize(mash::allocation_scope a2)
    {
        if (a2 == mash::ALLOCATED) {
            this->m_head = nullptr;
            this->m_size = 0;

            this->set_destruct_contents(true);
        }
    }

    void finalize(mash::allocation_scope);

    mAvlNode<value_type> *&root()
    {
        return this->m_head;
    }

    auto size() const
    {
        return this->m_size;
    }

    void set_destruct_contents(bool a2)
    {
        assert(!this->from_mash());
        this->field_C = a2;
    }

    bool get_destruct_contents() const
    {
        return this->field_C;
    }

    void sub_420EE0()
    {
        this->m_size = 0;
        this->field_0 = 0;
    }

    T *find(T *a1) const
    {
        auto v1 = findHelper(this->m_head, a1);
        if (v1 != nullptr) {
            return v1->m_key;
        }

        return nullptr;
    }

    node_type *findHelper(node_type *a1, T *a2) const
    {
        if (a1 == nullptr || a2 == nullptr) {
            return nullptr;
        }

        node_type node{a2};

        int v6 = compare(a1, &node);
        if (v6 != 0) {
            if (v6 <= 0) {
                return this->findHelper(a1->m_right, a2);
            } else {
                return this->findHelper(a1->m_left, a2);
            }
        }

        return a1;
    }

    void nodeHt(mAvlNode<T> *a2)
    {
        auto v2 = this->calcHeight(a2->m_right);
        auto v3 = this->calcHeight(a2->m_left);
        a2->m_height = std::max(v3, v2) + 1;
    }

    void singleRotateLeft(mAvlNode<T> *&a1);

    void doubleRotateLeft(mAvlNode<T> *&a1);

    void singleRotateRight(mAvlNode<T> *&a1);

    void doubleRotateRight(mAvlNode<T> *&a2);

    int *sub_64A090(int *a2);

    void sub_64A2B0();

    //0x0056DF00 -> T = T = string_hash_entry
    bool insert(T *new_element)
    {
        assert(new_element != nullptr);

        node_type *v4 = new node_type{new_element};

        int v5 = this->addHelper(v4, this->m_head, nullptr);
        bool v6 = (v5 != 0);
        if (v5 == 0 && v4 != nullptr) {
            delete v4;
        }

        return v6;
    }

    int addHelper(node_type *a2, node_type *&a3, node_type *a4);

    void unmash(mash_info_struct *a2, void *a3);

    mAvlTree<T>::iterator begin()
    {
        auto *node = this->m_head;
        if (node != nullptr) {
            while (node->m_left != nullptr) {
                node = node->m_left;
            }
        }

        return {node};
    }

    mAvlTree<T>::iterator end()
    {
        return {nullptr};
    }

    //0x005707B0
    void insert_tree(mAvlTree<T> *a2)
    {
        if constexpr (1) {
            auto destruct_contents = a2->get_destruct_contents();
            mAvlTree<T> v6{};

            auto v5 = a2->begin();
            auto end = a2->end();
            while (v5 != end) {
                auto &v2 = (*v5);
                if (!this->insert(v2.m_key) && destruct_contents) {
                    auto double_check = v6.insert(v2.m_key);
                    assert(double_check);
                }
            }

            if (destruct_contents) {
                v6.dump(v6.m_head);
            }
        } else {
            THISCALL(0x005707B0, this, a2);
        }
    }

    void dump(mAvlNode<value_type> *&node);

    void destruct_mashed_class();

    void destroy_element(mAvlNode<value_type> **p_node);

    static int compare(const mAvlNode<value_type> *a1, const mAvlNode<value_type> *a2)
    {
        if ((*a1->m_key) > (*a2->m_key)) {
            return 1;
        }

        if ((*a1->m_key) < (*a2->m_key)) {
            return -1;
        }

        return 0;
    }

    int8_t calcHeight(mAvlNode<value_type> *a1) const
    {
        return (a1 != nullptr ? a1->m_height : -1);
    }
};

extern void mAvlTree_patch();
