#include "resource_pack_directory.h"
#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

VALIDATE_SIZE(resource_pack_directory, 0x5C);

resource_pack_directory::resource_pack_directory()
{
    if constexpr (1)
    {
        this->field_0 = nullptr;
    }
    else
    {
        THISCALL(0x0052A9A0, this);
    }
}

void resource_pack_directory::set_resource_directory(resource_directory *directory)
{
    this->field_0 = directory;

    this->field_4.set_resource_directory(this->field_0);
    this->field_C.set_resource_directory(this->field_0);
    this->field_14.set_resource_directory(this->field_0);
    this->field_1C.set_resource_directory(this->field_0);
    this->field_24.set_resource_directory(this->field_0);
    this->field_2C.set_resource_directory(this->field_0);
    this->field_34.set_resource_directory(this->field_0);
    this->field_3C.set_resource_directory(this->field_0);
    this->field_44.set_resource_directory(this->field_0);
    this->field_4C.set_resource_directory(this->field_0);
    this->field_54.set_resource_directory(this->field_0);
}

void resource_pack_directory::clear()
{
    this->field_0 = nullptr;

    this->field_4.clear();
    this->field_C.clear();
    this->field_14.clear();
    this->field_1C.clear();
    this->field_24.clear();
    this->field_2C.clear();
    this->field_34.clear();
    this->field_3C.clear();
    this->field_44.clear();
    this->field_4C.clear();
    this->field_54.clear();
}

void resource_pack_directory_patch() {}
