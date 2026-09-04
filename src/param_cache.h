#pragma once

#include "float.hpp"

namespace als {

struct param;

struct param_cache {
    param *field_0;
    int field_4;

    int get_num_params_in_cache() const
    {
        if (this->field_4 <= 15) {
            return this->field_4;
        } else {
            return 15;
        }
    }

    void clear_cache();

    float cache_param(int a2, Float a3);

    float get_from_cache(int location) const;

    static inline constexpr auto INVALID_CACHE_ID = -1;
};
}  // namespace als
