#pragma once

#include <cstdint>
#include <cassert>

template <typename T, int Num>
struct simple_queue {
    static inline auto constexpr size = Num;

    T field_0[Num];
    short field_4;
    short field_6;
    short m_count;
    short field_A;

    short count() const
    {
        return m_count;
    }

    int get_unused_capacity() const
    {
        return size - this->m_count;
    }

    short push_back(const int &a2)
    {
        assert(this->m_count < size);

        if (++this->field_6 >= 32) {
            this->field_6 = 0;
        }

        this->field_0[this->field_6] = a2;
        return ++this->m_count;
    }

    int find(const T &a2, int16_t a3)
    {
        auto v4 = this->m_count;
        int result = this->field_4;
        if (v4 > 0) {
            if (this->field_0[result] == a2) {
                if (a3) {
                    this->field_4 = ++result;
                    if (result >= 16) {
                        this->field_4 = 0;
                    }

                    this->m_count = v4 - 1;
                }

                result = true;
            } else {
                auto v5 = this->field_6;
                if (result == v5) {
                LABEL_14:
                    result = 0;
                } else {
                    while (1) {
                        if (++result >= 16) {
                            result = 0;
                        }

                        if (this->field_0[result] == a2) {
                            break;
                        }

                        if (result == this->field_6) {
                            goto LABEL_14;
                        }
                    }

                    if (a3) {
                        result = result;
                        this->field_0[result] = this->field_0[v5];
                        --this->m_count;
                        if (--this->field_6 < 0) {
                            this->field_6 = 15;
                        }
                    }

                    result = true;
                }
            }
        } else {
            result = false;
        }

        return result;
    }

    int pop_front()
    {
        auto v2 = this->field_0[this->field_4];

        assert(m_count > 0);

        if (++this->field_4 >= 32)
            this->field_4 = 0;

        --this->m_count;
        return v2;
    }
};

extern void binary_search_array_unit_test();
