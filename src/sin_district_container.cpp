#include "sin_district_container.h"

#include "common.h"
#include "parse_generic_mash.h"

VALIDATE_SIZE(sin_district_container, 0x20);

sin_district_container::sin_district_container()
{

}

void sin_district_container::un_mash(
        generic_mash_header *,
        void *,
        generic_mash_data_ptrs *a4)
{
    a4->rebase(4);

    auto v5 = *a4->get<int>();
    this->district_name = a4->get<char>(v5);
}
