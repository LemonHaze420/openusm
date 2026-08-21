#pragma once

namespace _std {

template <typename T>
struct set {
    using value_type = T;
    struct node_t {
        node_t *_Left;
        node_t *_Parent;
        node_t *_Right;
        value_type _Myval;
        char _Color;
        char _Isnil;
    };

    struct iterator {
        node_t *m_ptr;

        bool operator==(const iterator &it) const
        {
            return this->m_ptr == it.m_ptr;
        }

        bool operator!=(const iterator &it) const
        {
            return this->m_ptr != it.m_ptr;
        }

        auto &operator*()
        {
            return this->m_ptr->_Myval;
        }

        void operator++()
        {
            void(__fastcall * func)(void *) = (decltype(func))0x00564160;
            func(this);
        }
    };

    using ret_t = std::pair<iterator, bool>;

    set()
    {
        node_t *v2 = nullptr;
        if constexpr (0) {
            node_t *(*sub_5E3E50)() = (decltype(sub_5E3E50))0x005E3E50;
            v2 = sub_5E3E50();
        } else {
            v2 = new node_t{};
        }

        this->m_head = v2;
        this->m_head->_Isnil = true;
        this->m_head->_Parent = this->m_head;
        this->m_head->_Left = this->m_head;
        this->m_head->_Right = this->m_head;
        this->m_size = 0;
    }

    auto size() const
    {
        return m_size;
    }

    iterator begin()
    {
        return iterator{this->m_head->_Left};
    }

    iterator end()
    {
        return iterator{this->m_head};
    }

    ret_t insert(const value_type &val)
    {
        ret_t result;
        void(__fastcall * func)(void *, void *, ret_t *, const value_type *) = (decltype(func))0x005B50E0;
        func(this, nullptr, &result, &val);
        return result;
    }

    iterator erase(iterator where)
    {
        iterator it;
        void(__fastcall * func)(void *, void *, iterator *, iterator) = (decltype(func))0x005B53F0;
        func(this, nullptr, &it, where);
        return it;
    }

    void clear()
    {
        void(__fastcall * func)(void *, void *, node_t *) = (decltype(func))0x005B3CD0;
        func(this, nullptr, this->m_head->_Parent);
        this->m_head->_Parent = this->m_head;
        this->m_size = 0;
        this->m_head->_Left = this->m_head;
        this->m_head->_Right = this->m_head;
    }

    int field_0;
    node_t *m_head;
    int m_size;
};

}  // namespace _std
