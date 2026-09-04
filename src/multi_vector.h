#pragma once

#include "list.hpp"
#include "vector.hpp"

template <typename T>
struct multi_vector {
    _std::list<_std::vector<T>> field_0;

    struct iterator {
        multi_vector<T> *field_0;
        typename _std::list<_std::vector<T>>::iterator field_4;
        typename _std::vector<T>::iterator field_8;

        iterator() = default;

        iterator(multi_vector *a2, typename _std::list<_std::vector<T>>::iterator &a3,
                 typename _std::vector<T>::iterator &a4)
        {
            this->field_0 = a2;
            this->field_4 = a3;
            this->field_8 = a4;
        }

        bool operator==(const multi_vector<T>::iterator &a2) const
        {
            if (this->field_0->empty() && a2.field_0->empty()) {
                return true;
            }

            return this->field_4 == a2.field_4 && this->field_8 == a2.field_8;
        }

        bool operator!=(const multi_vector<T>::iterator &it) const
        {
            return !(*this == it);
        }

        auto &operator*()
        {
            return (*this->field_8);
        }

        iterator &operator++()
        {
            auto v1 = this->field_0->field_0.end();
            if (this->field_4 != v1) {
                auto &v2 = (*this->field_4);
                auto v3 = v2.end();
                if (this->field_8 != v3) {
                    ++this->field_8;
                }

                while (1) {
                    auto &v4 = (*this->field_4);
                    auto v5 = v4.end();
                    if (!(this->field_8 == v5)) {
                        break;
                    }

                    auto v6 = this->field_0->field_0.end();
                    if (!(this->field_4 != v6)) {
                        break;
                    }

                    ++this->field_4;
                    auto v7 = this->field_0->field_0.end();
                    if (!(this->field_4 != v7)) {
                        break;
                    }

                    auto &v8 = (*this->field_4);
                    auto v9 = v8.begin();
                    this->field_8 = v9;
                }
            }

            return (*this);
        }
    };

    multi_vector() = default;

    iterator begin()
    {
        auto v12 = this->field_0.end();
        auto list_iterator = this->field_0.begin();

        typename _std::vector<T>::iterator vector_iter{};

        if (list_iterator != v12) {
            auto &v2 = (*list_iterator);
            vector_iter = v2.begin();
            while (list_iterator != v12) {
                auto &v3 = (*list_iterator);
                auto v4 = v3.end();
                if (vector_iter != v4) {
                    break;
                }

                ++list_iterator;
                if (list_iterator != v12) {
                    auto &v5 = (*list_iterator);
                    vector_iter = v5.begin();
                }
            }
        }

        iterator it{this, list_iterator, vector_iter};

        return it;
    }

    iterator end()
    {
        auto list_iter = this->field_0.end();

        typename _std::vector<T>::iterator vector_iter{};
        if (!this->empty()) {
            auto v2 = this->field_0.end();
            auto &v3 = *(--v2);
            vector_iter = v3.end();
        }

        iterator it{this, list_iter, vector_iter};

        return it;
    }

    bool empty()
    {
        return field_0.empty();
    }

    _std::vector<T> *sub_50A2B0();

    _std::vector<T> *sub_50A230();

    //0x0056C050
    int get_vector_index(_std::vector<T> *a2)
    {
        auto *v2 = this->field_0.m_head;
        auto *v3 = v2->_Next;
        int result = 0;
        if (v2->_Next == v2) {
            return -1;
        }

        while (&v3->_Myval != a2) {
            v3 = v3->_Next;
            ++result;
            if (v3 == v2) {
                return -1;
            }
        }

        return result;
    }

    bool sub_572FB0(_std::vector<T> *a2)
    {
        auto *v2 = this->field_0.m_head;
        auto *v3 = v2->_Next;
        if (v2->_Next == v2) {
            return false;
        }

        while (&v3->_Myval != a2) {
            v3 = v3->_Next;
            if (v3 == v2) {
                return false;
            }
        }

        this->field_0.erase({v3});

        return true;
    }
};
