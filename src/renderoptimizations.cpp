#include "renderoptimizations.h"

#include "common.h"

VALIDATE_SIZE(RenderOptimizations, 0x30u);

RenderOptimizations::RenderOptimizations()
{
    init_defaults();
}

void RenderOptimizations::init_defaults()
{
    this->field_1 = false;
    this->field_0 = true;
    this->field_4 = 35.0f;
    this->field_9 = false;
    this->field_C = 0;
    this->field_10 = 0;
    this->field_14 = 0;
    this->field_18 = 0;
    this->field_1C = false;
    this->field_20 = 0;
    this->field_24 = 0;
    this->field_28 = 0;
    this->field_2C = 0;
}
