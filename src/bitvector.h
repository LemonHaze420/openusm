#pragma once

#include <cassert>
#include <cstdint>

#include "bitmath.h"
#include "variable.h"

inline constexpr auto LOG_BASE_TYPE_BITS = 5;

template <typename base_type, int number_of_bits>
struct fixed_bitvector {
    static constexpr auto num = number_of_bits / (sizeof(base_type) * 8) + 1;
    int field_0;
    base_type field_4[num]{};

    fixed_bitvector()
    {
        assert(LOG_BASE_TYPE_BITS == bitmath::intlog2(sizeof(base_type) * 8));

        this->field_0 = number_of_bits + sizeof(base_type) * 8;

        auto sub_A62040 = [](auto *self) -> void {
            for (auto i = 0u; i < 65u; ++i) {
                self->field_4[i] = 0;
            }
        };

        sub_A62040(this);
    }

    void clear()
    {
        for (auto i = 0u; i < num; ++i) {
            this->field_4[i] = 0;
        }
    }

    int get_index(int a1)
    {
        return a1 >> 5;
    }

    int get_bit(uint8_t a1)
    {
        return 1 << (a1 & 31);
    }

    void set(int bit_position)
    {
        assert(bit_position >= 0 && bit_position < number_of_bits);

        auto &v2 = this->field_4[get_index(bit_position)];
        v2 |= get_bit(bit_position);
    }
};

inline Var<fixed_bitvector<uint32_t, 256>> hash_update_bitvector{0x009222B0};
