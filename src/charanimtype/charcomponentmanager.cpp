#include "charcomponentmanager.h"

void CharComponentManager::RegisterComponent(CharComponentBase *a1)
{
    if ( pCompArray == nullptr )
    {
        pCompArray = (BaseComponent **)operator new(0xC8u);
        memset(pCompArray, 0, 0xC8u);
    }
    
    pCompArray[iCurrNumComponents++] = (BaseComponent *)a1;
}
