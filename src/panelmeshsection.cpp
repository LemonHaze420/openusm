#include "panelmeshsection.h"

#include "common.h"
#include "ngl_mesh.h"
#include "ngl.h"
#include "trace.h"
#include "func_wrapper.h"

VALIDATE_SIZE(PanelMeshSection, 0x50);

PanelMeshSection::PanelMeshSection() {}

void PanelMeshSection::SetMesh(nglMesh *a2)
{
    TRACE("PanelMeshSection::SetMesh");

#ifdef TARGET_XBOX
    this->field_40 = a2;
    this->field_40->SphereRadius = 0.0000099999997;
    for (int i = this->field_40->NSections; --i >= 0;
         this->field_40->Sections[i].Section->SphereRadius = 0.0000099999997) {
        ;
    }
#else
    THISCALL(0x00607F20, this, a2);
#endif
}

void PanelMeshSection_patch()
{
    {
        FUNC_ADDRESS(address, &PanelMeshSection::SetMesh);
        REDIRECT(0x00637F96, address);
    }
}
