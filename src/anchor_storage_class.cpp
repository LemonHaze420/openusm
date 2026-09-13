#include "anchor_storage_class.h"

#include "func_wrapper.h"

anchor_storage_class::anchor_storage_class() {}

anchor_storage_class::anchor_storage_class(vhandle_type<entity> a2, entity_base_vhandle a3)
{
    this->field_0 = a2;
    this->field_4 = a3.field_0;
}

bool anchor_storage_class::is_valid() const
{
    return this->field_0.get_volatile_ptr() != nullptr;
}

vector3d anchor_storage_class::get_origin() const
{
    void *(__fastcall * func)(const void *, int, vector3d *) = CAST(func, 0x00464480);

    vector3d result;
    func(this, 0, &result);
    return result;
}

vector3d anchor_storage_class::get_target() const
{
    vector3d result;
    THISCALL(0x00464370, this, &result);

    return result;
}
