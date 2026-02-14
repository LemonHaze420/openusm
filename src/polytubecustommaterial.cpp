#include "polytubecustommaterial.h"

#include "common.h"

VALIDATE_SIZE(PolytubeCustomMaterial, 0x30);

PolytubeCustomMaterial::PolytubeCustomMaterial(nglTexture *a2, nglBlendModeType a3, int a5)
    : PCUV_ShaderMaterial(a2, a3, 0, a5) {
    this->field_1C = &a2->FileName;
    this->m_vtbl = 0x0087E698;
}
