#include "force_transitions.h"

#include "alter_conditions.h"
#include "common.h"
#include "mash_info_struct.h"

namespace als {
VALIDATE_SIZE(force_transitions, 0x18);

force_transitions::force_transitions() : field_0(), field_4() {}

force_transitions::force_transitions(from_mash_in_place_constructor *a2) : field_0(a2), field_4(a2) {}

void force_transitions::unmash(mash_info_struct *a1, void *)
{
    a1->unmash_class_in_place(this->field_0, this);
    a1->unmash_class_in_place(this->field_4, this);
}
}  // namespace als
