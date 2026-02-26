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
    if constexpr (1)
    {
        this->m_meshSection = a3;
        this->m_meshNode = a2;
        this->m_vtbl = 0x00871D28;

        auto *param_set = &a2->field_8C;
        if (param_set->IsSetParam<USSectionIFLParam>())
        {
            USSectionIFLParam default_param{};
            auto *param = param_set->GetOrDefault<USSectionIFLParam>(default_param);

            auto *info = param->field_0;
            assert(info != nullptr);

            this->field_14 = info->CurrentSection;

            ++info->CurrentSection;

        }
        else
        {
            this->field_14 = -1;
        }

    } else {
        THISCALL(0x0041BDA0, this, a2, a3);
    }
}

nglTexture *USVariantShaderNode::ResolveIFL(nglTexture *a2)
{
    if constexpr (1)
    {
        if (LOBYTE(a2->m_format) != 16) {
            return a2;
        }

        auto &param_set = this->m_meshNode->field_8C;

        uint32_t idx = -1;

        if (param_set.IsSetParam<USSectionIFLParam>())
        {
            USSectionIFLParam default_param{};
            auto *param = param_set.GetOrDefault<USSectionIFLParam>(default_param);

            auto *info = param->field_0;

            assert(info != nullptr);

            auto sub_6F6FE0 = [](auto *self, unsigned int Section) -> char {
                assert(Section < self->NSections);

                return self->field_8[Section];
            };

            idx = sub_6F6FE0(info, this->field_14);
        }

        if (idx != static_cast<uint32_t>(-1)) {
            return a2->Frames[idx % a2->m_num_palettes];
        }

        if (param_set.IsSetParam<nglTextureFrameParam>()) {
            auto *param = param_set.Get<nglTextureFrameParam>();

            idx = param->field_0;
        } else {
            idx = nglCurScene->IFLFrame;
        }

        return a2->Frames[idx % a2->m_num_palettes];

    }
    else
    {
        return (nglTexture *) THISCALL(0x0041BE30, this, a2);
    }
}

double USVariantShaderNode::sub_415D10() {
    vector4d v3, v4;

    CDECL_CALL(0x00414360, &v3, &this->m_meshSection->SphereCenter, this->m_meshNode);
    CDECL_CALL(0x00414360, &v4, &v3, &nglCurScene->WorldToView);
    return v4[2] + this->m_meshSection->SphereRadius;
}

double USVariantShaderNode::sub_41DEA0() const {
    vector4d v2;
    vector4d v3;

    CDECL_CALL(0x00414360, &v2, &this->m_meshNode->Mesh->SphereCenter, this->m_meshNode);
    CDECL_CALL(0x00414360, &v3, &v2, &nglCurScene->WorldToView);
    return v3[2];
}

float USVariantShaderNode::GetDistanceScale() const
{
    TRACE("USVariantShaderNode::GetDistanceScale");

    if constexpr (1)
    {
        float v4 = this->sub_41DEA0();
        if (v4 > 20.0f) {
            return 0.0f;
        }

        return std::clamp(v4, 1.0f, 10.0f) * g_tan_half_fov_ratio;
    }
    else
    {
        float (__fastcall *func)(const void *) = CAST(func, 0x0041DE40);
        auto result = func(this);

        return result;
    }
}

void USVariantShaderNode_patch() {
    {
        FUNC_ADDRESS(address, &USVariantShaderNode::ResolveIFL);
        SET_JUMP(0x0041BE30, address);
    }
}
