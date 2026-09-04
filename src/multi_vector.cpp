#include "multi_vector.h"

#include "entity.h"
#include "func_wrapper.h"
#include "item.h"

template <>
_std::vector<item *> *multi_vector<item *>::sub_50A2B0()
{
    return (_std::vector<item *> *)THISCALL(0x0050A2B0, this);
}

template <>
_std::vector<entity *> *multi_vector<entity *>::sub_50A230()
{
    return (_std::vector<entity *> *)THISCALL(0x0050A230, this);
}
