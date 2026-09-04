#include "city_lights.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "parse_generic_mash.h"
#include "trace.h"

VALIDATE_SIZE(city_lights, 0x24);

void city_lights::load_from_file(const mString &a2)
{
    THISCALL(0x0051B5C0, this, &a2);
}

void city_lights::update(Float a1, nglMatrix &a2, nglMatrix &a3)
{
    THISCALL(0x00527EB0, this, a1, &a2, &a3);
}

void city_lights::un_mash_start(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3, void *)
{
    this->un_mash(a1, a2, a3);
}

void city_lights::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4)
{
    TRACE("city_lights::un_mash");

    if constexpr (1) {
        a4->rebase_shared(16u);

        a4->rebase_shared(4u);

        this->field_0 = a4->get_from_shared<char>(3072);
        a4->rebase_shared(4u);

        this->field_4 = a4->get_from_shared<char>(3072);
        a4->rebase_shared(4u);

        this->field_8 = a4->get_from_shared<char>(768);
    } else {
        THISCALL(0x0051B520, this, a2, a3, a4);
    }
}

void city_lights_patch()
{
    FUNC_ADDRESS(address, &city_lights::un_mash);
    REDIRECT(0x005507E2, address);
}
