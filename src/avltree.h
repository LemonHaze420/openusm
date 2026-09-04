#pragma once

template <typename T>
struct TreeNode {
    TreeNode<T> *field_0;
    TreeNode<T> *field_4;
    TreeNode<T> *field_8;
    T *m_key;
    char field_10;
};

template <typename T>
struct AvlTree {
    TreeNode<T> *field_0;
    int m_size;
    bool field_8;

    auto size() const
    {
        return this->m_size;
    }

    int addHelper(TreeNode<T> *, TreeNode<T> *&, TreeNode<T> *);

    void dump(TreeNode<T> *&a2, int a3);

    TreeNode<T> *findHelper(TreeNode<T> *, T *) const;

    T *find(T *a2) const
    {
        auto *v3 = this->findHelper(this->field_0, a2);
        if (v3 != nullptr) {
            return v3->m_key;
        }

        return nullptr;
    }
};
