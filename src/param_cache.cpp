#include "param_cache.h"

#include "param_list.h"

#include <cassert>

namespace als {

void param_cache::clear_cache()
{
    this->field_4 = 0;
}

float param_cache::cache_param(int a2, Float a3)
{
    auto idx = this->field_4 % 15;
    auto &v5 = this->field_0;

    param p{a2, a3};
    v5[idx] = p;
    ++this->field_4;
    return a3;
}

float param_cache::get_from_cache(int location) const
{
    assert(location != INVALID_CACHE_ID && "Given item is not in the cache.");
    assert(location < get_num_params_in_cache() && "Given item is outside the range of the cache.");
    return this->field_0[location].field_4;
}

}  // namespace als
