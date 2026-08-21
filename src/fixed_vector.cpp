#include "fixed_vector.h"

#include "ai_find_best_swing_anchor.h"
#include "func_wrapper.h"
#include "quick_anchor_info.h"
#include "vector4d.h"

#include <cassert>

template <>
void fixed_vector<quick_anchor_info, 100>::sort()
{
    auto *begin = this->m_data;
    auto *end = &this->m_data[m_size];
    auto size = end - begin;

    ::sort(begin, end, size);
}

template <>
void fixed_vector<quick_anchor_info, 100>::resize(int size, const quick_anchor_info &a3)
{
    assert(unsigned(size) <= N);

    int v3 = this->m_size;
    if (v3 < size) {
        if (v3 < size) {
            auto v4 = size - v3;
            do {
                this->m_data[this->m_size] = a3;

                --v4;
                ++this->m_size;
            } while (v4);

            assert(static_cast<int>(this->m_size) == size);
        }
    } else {
        this->m_size = size;
    }
}
