#include "fetextflashinfo.h"

#include "common.h"

VALIDATE_SIZE(FETextFlashInfo, 0x10);

FETextFlashInfo::FETextFlashInfo(color32 a3, float a4)
{
    this->field_0 = a3;
    this->field_C = a4;
    this->field_4 = 0;
    this->field_8 = 0.0;
}

color32 FETextFlashInfo::GetColor(color32 a3)
{
    auto v3 = this->field_8 + 0.5f;

    color32 result;
    result[0] = ((this->field_0.field_0[0] - a3.field_0[0]) * v3 + a3.field_0[0]);
    result[1] = ((this->field_0.field_0[1] - a3.field_0[1]) * v3 + a3.field_0[1]);
    result[2] = ((this->field_0.field_0[2] - a3.field_0[2]) * v3 + a3.field_0[2]);
    result[3] = ((this->field_0.field_0[3] - a3.field_0[3]) * v3 + a3.field_0[3]);
    return result;
}
