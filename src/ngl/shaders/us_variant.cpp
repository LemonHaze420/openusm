#include "us_variant.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

VALIDATE_SIZE(USVariantShaderNode, 0x18);

USVariantShaderNode::USVariantShaderNode(nglMeshNode *a2, nglMeshSection *a3) : nglShaderNode(a2, a3)
{
    this->m_vtbl = 0x00871D28;
    auto &params = a2->field_8C;
    if (params.IsSetParam<USSectionIFLParam>()) {
        auto *info = params.Get<USSectionIFLParam>()->field_0;
        this->field_14 = info->CurrentSection++;
    } else {
        this->field_14 = -1;
    }
}

nglTexture *USVariantShaderNode::ResolveIFL(nglTexture *texture)
{
    if ((texture->m_format & 0xFF) != 16) {
        return texture;
    }

    auto &params = this->m_meshNode->field_8C;
    uint32_t frame = UINT32_MAX;
    if (params.IsSetParam<USSectionIFLParam>()) {
        auto *info = params.Get<USSectionIFLParam>()->field_0;

        frame = static_cast<uint32_t>(static_cast<int8_t>(info->field_8[this->field_14]));
    }
    if (frame == UINT32_MAX) {
        frame = params.IsSetParam<nglTextureFrameParam>()
                    ? static_cast<uint32_t>(params.Get<nglTextureFrameParam>()->field_0)
                    : static_cast<uint32_t>(nglCurScene->IFLFrame);
    }
    return texture->Frames[frame % texture->m_num_palettes];
}

double USVariantShaderNode::sub_415D10()
{
    const auto worldCenter = sub_414360(this->m_meshSection->SphereCenter, this->m_meshNode->LocalToWorld);
    const auto viewCenter = sub_414360(worldCenter, nglCurScene->WorldToView);
    return static_cast<double>(viewCenter[2]) + this->m_meshSection->SphereRadius;
}

double USVariantShaderNode::sub_41DEA0() const
{
    const auto worldCenter = sub_414360(this->m_meshNode->Mesh->SphereCenter, this->m_meshNode->LocalToWorld);
    const auto viewCenter = sub_414360(worldCenter, nglCurScene->WorldToView);
    return viewCenter[2];
}

float USVariantShaderNode::GetDistanceScale() const
{
    const double depth = this->sub_41DEA0();
    if (depth > 20.0) {
        return 0.0f;
    }

    const float distance = static_cast<float>(depth);

    if (!(distance > 1.0f)) {
        return g_tan_half_fov_ratio;
    }
    return (distance >= 10.0f ? 10.0f : distance) * g_tan_half_fov_ratio;
}

void USVariantShaderNode_patch()
{
    {
        FUNC_ADDRESS(address, &USVariantShaderNode::ResolveIFL);
        SET_JUMP(0x0041BE30, address);
    }
}
