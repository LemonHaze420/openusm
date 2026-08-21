#include "alter_conditions.h"

#include "mash_info_struct.h"
#include "common.h"

namespace als {
VALIDATE_SIZE(alter_conditions, 0xC);

alter_conditions::alter_conditions(from_mash_in_place_constructor *a2) : field_8(a2) {}

void alter_conditions::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_8, this);
}
}  // namespace als
