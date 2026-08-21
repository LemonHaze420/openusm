#include "instance_bank.h"

#include "colmesh.h"
#include "func_wrapper.h"

template <>
void instance_bank<cg_mesh>::purge()
{
    THISCALL(0x0056DDE0, this);
}
