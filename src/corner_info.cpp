#include "corner_info.h"

#include "common.h"

namespace ai {

VALIDATE_SIZE(corner_info, 0xA8);

corner_info::corner_info() {}

corner_info::corner_info(from_mash_in_place_constructor *constructor) : field_0(constructor)
{
    field_0.clear();
}

corner_info::corner_info(const corner_info &source)
    : field_0(static_cast<from_mash_in_place_constructor *>(nullptr))
{
    field_0.collision = false;
    field_0.field_59 = false;
    field_0.queued_for_collision_check = false;
    *this = source;
}

corner_info &corner_info::operator=(const corner_info &source)
{
    field_0.copy(source.field_0);
    for (unsigned i = 0; i != 19; ++i)
        field_5C[i] = source.field_5C[i];
    return *this;
}

void corner_info::clear()
{
    field_0.clear();
}

}  // namespace ai
