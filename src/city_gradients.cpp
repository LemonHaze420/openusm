#include "city_gradients.h"

#include "parse_generic_mash.h"
#include "common.h"
#include "resource_key.h"

VALIDATE_SIZE(city_gradients, 0x1C);

void city_gradients::un_mash_start(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4, void *)
{
   this->un_mash(a2, a3, a4);
}

void city_gradients::un_mash(generic_mash_header *, void *, generic_mash_data_ptrs *a4)
{
    a4->rebase_shared(16u);

    a4->rebase_shared(4u);

    struct internal_t {
        char field_0[0xD0];
    };

    this->field_10 = a4->get_from_shared<internal_t>(this->num_gradients);

    a4->rebase_shared(4u);

    struct type {
        char field_0[0x10];
    };

    this->field_14 = a4->get_from_shared<type>(this->num_gradients);

    a4->rebase_shared(4u);

    this->field_18 = a4->get_from_shared<resource_key>(this->num_gradient_keys);
}
