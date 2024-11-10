#include "mAvlTree.h"

#include "common.h"
#include "entity_class_entry.h"
#include "mash_info_struct.h"
#include "string_hash_entry.h"
#include "utility.h"

VALIDATE_SIZE(mAvlTree<string_hash_entry>, 16u);

template<>
void mAvlTree<string_hash_entry>::singleRotateRight(mAvlNode<string_hash_entry> *&k2)
{
    auto k1 = k2->m_left;
    //assert(k1->m_parent == k2);

    auto parent = k2->m_parent;
    k2->m_left = k1->m_right;
    if (k2->m_left != nullptr) {
        k2->m_left->m_parent = k2;
    }

    k1->m_right = k2;
    if (k1->m_right != nullptr) {
        k1->m_right->m_parent = k1;
    }

    k2->updateHeight(k2->m_left, k2->m_right);
    k1->updateHeight(k1->m_left, k2);

    k2 = k1;
    k2->m_parent = parent;
}

template<>
void mAvlTree<string_hash_entry>::singleRotateLeft(mAvlNode<string_hash_entry> *&k2)
{
    auto k1 = k2->m_right;
    auto parent = k2->m_parent;
    k2->m_right = k1->m_left;
    if (k1->m_left != nullptr) {
        k1->m_left->m_parent = k2;
    }

    k1->m_left = k2;
    if (k1->m_left != nullptr) {
        k1->m_left = k1;
    }

    k2->updateHeight(k2->m_left, k2->m_right);
    k1->updateHeight(k1->m_right, k2);

    k2 = k1;
    k2->m_parent = parent;
}

template<>
void mAvlTree<string_hash_entry>::doubleRotateLeft(mAvlNode<string_hash_entry> *&a1) {
    this->singleRotateRight(a1->m_right);
    this->singleRotateLeft(a1);
}

template<>
void mAvlTree<string_hash_entry>::doubleRotateRight(mAvlNode<string_hash_entry> *&a2) {
    this->singleRotateLeft(a2->m_left);
    this->singleRotateRight(a2);
}

template<>
int *mAvlTree<string_hash_entry>::sub_64A090(int *a2) {
    return (int *) THISCALL(0x0064A090, this, a2);
}

template<>
void mAvlTree<string_hash_entry>::sub_64A2B0() {
    THISCALL(0x0064A2B0, this);
}

//0x0056AE30
template<>
int mAvlTree<string_hash_entry>::addHelper(mAvlNode<string_hash_entry> *a2,
                                           mAvlNode<string_hash_entry> *&a3,
                                           mAvlNode<string_hash_entry> *a4)
{
    if constexpr (1)
    {
        int result;
        auto v4 = a3;
        auto &v5 = *a3;

        if (a3 == nullptr)
        {
            a2->m_parent = a4;
            a3 = a2;
            ++this->m_size;
            return 1;
        }

        auto v7 = compare(a2, a3);
        if (v7 > 0)
        {
            result = this->addHelper(a2, v5.m_right, &v5);
            auto a1a = this->calcHeight(a3->m_right);
            auto v19 = this->calcHeight(a3->m_left);

            if (a1a - v19 == 2)
            {
                if (compare(a2, a3->m_right) <= 0) {
                    this->doubleRotateLeft(v4);
                } else {
                    this->singleRotateLeft(v4);
                }

            } else {
                this->nodeHt(a3);
            }
        }
        else if (v7 < 0)
        {
            result = this->addHelper(a2, v5.m_left, &v5);
            auto v15 = a3->heightDiff();
            if (v15 == 2)
            {
                if ( compare(a2, a3->m_left) >= 0 ) {
                    this->doubleRotateRight(v4);
                } else {
                    this->singleRotateRight(v4);
                }

            } else {
                this->nodeHt(a3);
            }
        }
        else
        {
            result = v7;
        }

        return result;
    } else {
        return THISCALL(0x0056AE30, this, a2, a3, a4);
    }
}

template<>
void mAvlTree<string_hash_entry>::destroy_element(mAvlNode<string_hash_entry> **p_node)
{
    TRACE("mAvlTree<string_hash_entry>::destroy_element");

    if constexpr (0)
    {
        if ( this->field_C )
        {
            if ( this->is_pointer_in_mash_image((*p_node)->m_key) ) {
                (*p_node)->m_key->destruct_mashed_class();
            } else if ((*p_node)->m_key != nullptr) {
                delete ((*p_node)->m_key);
            }
        }

        (*p_node)->m_key = nullptr;
        if ( this->is_pointer_in_mash_image(*p_node) ) {
            (*p_node)->destruct_mashed_class();
        } else if ( (*p_node) != nullptr ) {
            delete (*p_node);
        }

        (*p_node) = nullptr;
    }
    else
    {
        THISCALL(0x00567350, this, p_node);
    }
}

template<>
void mAvlTree<string_hash_entry>::dump(mAvlNode<string_hash_entry> *&node)
{
    TRACE("mAvlTree<string_hash_entry>::dump");

    if constexpr (1)
    {
        if (node != nullptr)
        {
            this->dump(node->m_left);
            this->dump(node->m_right);
            this->destroy_element(&node);
            --this->m_size;
        }
    } else {
        THISCALL(0x0056A990, this);
    }
}

template<>
void mAvlTree<string_hash_entry>::finalize(mash::allocation_scope) {
    TRACE("mAvlTree<entity_class_entry>::finalize");
    this->dump(this->m_head);
}

template<>
void mAvlTree<string_hash_entry>::destruct_mashed_class()
{
    TRACE("mAvlTree<string_hash_entry>::destruct_mashed_class");

    if constexpr (1) {
        this->finalize(mash::FROM_MASH);
        mContainer_base::destruct_mashed_class();
    } else {
        THISCALL(0x0056F8B0, this);
    }
}

template<>
void mAvlTree<entity_class_entry>::dump(mAvlNode<entity_class_entry> *&node)
{
    TRACE("mAvlTree<entity_class_entry>::dump");

    if constexpr (0) {
        if (node != nullptr)
        {
            this->dump(node->m_left);
            this->dump(node->m_right);
            this->destroy_element(&node);
            --this->m_size;
        }
    } else {
        THISCALL(0x007483D0, this, &node);
    }
}

template<>
void mAvlTree<entity_class_entry>::finalize(mash::allocation_scope) {
    TRACE("mAvlTree<entity_class_entry>::finalize");
    this->dump(this->m_head);
}

template<>
void mAvlTree<string_hash_entry>::unmash(mash_info_struct *a2, [[maybe_unused]] void *a3)
{
    if constexpr (1)
    {
        this->m_head = nullptr;
        auto real_size = this->m_size;
        this->m_size = 0;

        for (auto i = 0; i < real_size; ++i)
        {
            mAvlNode<string_hash_entry> *v7 = nullptr;
            a2->unmash_class(v7, this);
            this->addHelper(v7, this->m_head, nullptr);
        }

        assert(real_size == this->m_size);

        this->field_0 = (int) &a2->mash_image_ptr[a2->buffer_size_used[0] - (DWORD) this];
    }
    else
    {
        THISCALL(0x00570900, this, a2, a3);
    }
}

template<>
void mAvlNode<string_hash_entry>::unmash(
                            mash_info_struct *a3,
                            void *)
{
    a3->unmash_class(this->m_key, this);
}

void __fastcall string_hash_entry__dtor(string_hash_entry *self) {
    TRACE("string_hash_entry::~string_hash_entry");

    self->field_4.~mString();
}

void __cdecl operator_delete(void *a1)
{
    TRACE("operator delete");
    sp_log("0x%08X", int(a1));

    CDECL_CALL(0x0082207C, a1);
}

void mAvlTree_patch()
{
    {
        auto address = &string_hash_entry__dtor;
        REDIRECT(0x00567386, address);
    }

    {
        auto address = operator_delete;
        REDIRECT(0x0056738C, address);
    }

    {
        FUNC_ADDRESS(address, &mAvlTree<string_hash_entry>::findHelper);
        REDIRECT(0x00531A07, address);
    }

    {
        FUNC_ADDRESS(address, &mAvlTree<string_hash_entry>::iterator::iterate);
        //REDIRECT(0x0052A7C2, address);
    }

    {
        FUNC_ADDRESS(address, &mAvlTree<string_hash_entry>::unmash);
        REDIRECT(0x00550F44, address);
    }

    {
        FUNC_ADDRESS(address, &mAvlTree<string_hash_entry>::destruct_mashed_class);
        REDIRECT(0x00547A52, address);
    }
}
