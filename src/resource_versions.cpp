#include "resource_versions.h"

mString resource_versions::to_string() const
{
    mString result = mString{
        mString::fmtd{0}, "%d.%d.%d.%d.%d", this->field_0, this->field_4, this->field_8, this->field_C, this->field_10};
    return result;
}
