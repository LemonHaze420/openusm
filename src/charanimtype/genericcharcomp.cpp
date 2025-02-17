#include "genericcharcomp.h"

#include "charcomponentmanager.h"
#include "common.h"

GenericCharComp::GenericCharComp()
{
    this->m_vtbl = 0x00892140;
    this->m_strTypeString = "Generic";
    CharComponentManager::RegisterComponent(this);
}
