#include "als_post_layer_alter.h"

#include "alter_conditions.h"
#include "mash_info_struct.h"
#include "common.h"

namespace als
{
    VALIDATE_SIZE(post_layer_alter, 0x1Cu);

    post_layer_alter::post_layer_alter(from_mash_in_place_constructor *a2) : field_4(a2), field_18(a2) {}

    void post_layer_alter::unmash(mash_info_struct *a1, void *)
    {
        a1->unmash_class_in_place(this->field_4, this);
        a1->unmash_class_in_place(this->field_18, this);
    }
}
