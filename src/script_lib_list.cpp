#include "script_lib_list.h"

#include "func_wrapper.h"

#if !STANDALONE_SYSTEM
void destroy_script_lists()
{
    CDECL_CALL(0x00661AE0);
}
#endif
