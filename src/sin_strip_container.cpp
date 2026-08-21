#include "sin_strip_container.h"

#include "common.h"
#include "parse_generic_mash.h"

VALIDATE_SIZE(sin_strip_container, 0x10);

sin_strip_container::sin_strip_container() {}

void sin_strip_container::un_mash(generic_mash_header *a2, void *, generic_mash_data_ptrs *a4)
{
    a4->rebase(4u);

    auto v5 = *a4->get<int>();
    this->strip_name = a4->get<char>(v5);

    this->field_4.un_mash(a2, &this->field_4, a4, nullptr);
}
