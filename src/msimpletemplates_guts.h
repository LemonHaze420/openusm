#pragma once

#include <cstdint>

template<typename T>
struct simple_list {
    T *_first_element;
    T *_last_element;
    uint32_t m_size;

    struct vars_t {
        T *_sl_next_element;
        T *_sl_prev_element;
        simple_list<T> *_sl_list_owner;
    };

    struct iterator {
        T *_Ptr;

        bool operator==(const iterator &it) const {
            return this->_Ptr == it._Ptr;
        }

        bool operator!=(const iterator &it) const {
            return this->_Ptr != it._Ptr;
        }

        void operator++() {
            if ( this->_Ptr != nullptr ) {
                this->_Ptr = this->_Ptr->simple_list_vars._sl_next_element;
            }
        }

        T &operator*() {
            return (*this->_Ptr);
        }
    };

    uint32_t size() const {
        return this->m_size;
    }

    bool empty() const {
        return (this->m_size == 0);
    }

    iterator begin() {
        return iterator {this->_first_element};
    }
    
    iterator end() {
        return iterator {nullptr};
    }

    iterator push_back(T *tmp) {
        assert(tmp != nullptr);

        assert(tmp->simple_list_vars._sl_next_element == nullptr);

        assert(tmp->simple_list_vars._sl_prev_element == nullptr);

        assert(tmp->simple_list_vars._sl_list_owner == nullptr);

        tmp->simple_list_vars._sl_next_element = this->_first_element;
        tmp->simple_list_vars._sl_prev_element = nullptr;
        if ( this->_first_element != nullptr ) {
            this->_first_element->simple_list_vars._sl_prev_element = tmp;
        }

        this->_first_element = tmp;
        if ( tmp->simple_list_vars._sl_next_element == nullptr ) {
            this->_last_element = tmp;
        }

        tmp->simple_list_vars._sl_list_owner = this;
        ++this->m_size;
        iterator a2{tmp};
        return a2;
    }

    iterator emplace_back(T *tmp) {
        assert(tmp != nullptr);

        assert(tmp->simple_list_vars._sl_next_element == nullptr);
        
        assert(tmp->simple_list_vars._sl_prev_element == nullptr);

        assert(tmp->simple_list_vars._sl_list_owner == nullptr);

        if ( this->_last_element != nullptr ) {

            assert(_last_element->simple_list_vars._sl_next_element == nullptr);

            this->_last_element->simple_list_vars._sl_next_element = tmp;
            tmp->simple_list_vars._sl_prev_element = this->_last_element;
            tmp->simple_list_vars._sl_next_element = nullptr;
            this->_last_element = tmp;
            tmp->simple_list_vars._sl_list_owner = this;
            ++this->m_size;
            return iterator {tmp};
        }
        else
        {
            return this->push_back(tmp);
        }
    }

    bool contains(iterator a2) const {
        return a2._Ptr != nullptr && a2._Ptr->simple_list_vars._sl_list_owner == this;
    }

    T *common_erase(T *iter, bool a3) {
        T *result = nullptr;
        if ( iter != nullptr ) {
            assert(this->contains(iterator {iter}));

            result = (a3
                    ? iter->simple_list_vars._sl_prev_element
                    : iter->simple_list_vars._sl_next_element
                    );

            if ( iter->simple_list_vars._sl_prev_element != nullptr ) {
                iter->simple_list_vars._sl_prev_element = iter->simple_list_vars._sl_next_element;
            } else {
                assert(iter->simple_list_vars._sl_list_owner->_first_element == iter);

                iter->simple_list_vars._sl_list_owner->_first_element = iter->simple_list_vars._sl_next_element;
            }

            if ( iter->simple_list_vars._sl_next_element != nullptr ) {
                iter->simple_list_vars._sl_next_element->simple_list_vars._sl_prev_element = iter->simple_list_vars._sl_prev_element;
            } else {
                assert(iter->simple_list_vars._sl_list_owner->_last_element == iter);

                iter->simple_list_vars._sl_list_owner->_last_element = iter->simple_list_vars._sl_prev_element;
            }

            assert(iter->simple_list_vars._sl_list_owner->m_size >= 0);

            iter->simple_list_vars._sl_next_element = nullptr;
            iter->simple_list_vars._sl_prev_element = nullptr;
            iter->simple_list_vars._sl_list_owner = nullptr;
        }

        return result;
    }

    bool erase(iterator a2) {
        if ( !this->contains(a2) ) {
            return false;
        }

        this->common_erase(a2._Ptr, false);
        return true;
    }

};
