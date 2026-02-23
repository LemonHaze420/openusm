#include "us_shaderbase.h"

#include "trace.h"

bool USShaderBase::_IsSwitchable() const
{
    TRACE("USShaderBase::IsSwitchable");

    return false;
}
