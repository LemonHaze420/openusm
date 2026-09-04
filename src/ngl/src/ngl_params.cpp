#include "ngl_params.h"

#include "color.h"
#include "common.h"
#include "ngl.h"

VALIDATE_SIZE(nglTintParam, 0x4);

Var<nglParam> EmptyParam{0x00971E80};

Var<int> nglShaderParamSet_Pool::NextID = {0x00971E8C};

Var<int> nglSceneParamSet_Pool::NextID = (0x00971E88);

template <>
void nglParamSet<nglShaderParamSet_Pool>::set_color(color32 a2)
{
    const color32 white{255, 255, 255, 255};
    if (a2 != white) {
        color v9 = a2.to_color();
        auto *mem = nglListAlloc(16, 16);
        vector4d *v4 = new (mem) vector4d{v9.r, v9.g, v9.b, v9.a};

        nglTintParam param{v4};

        this->SetParam(param);
    }
}

nglMaterialBase *sub_8EA2E0(nglParamSet<nglShaderParamSet_Pool> *a1, nglMaterialBase *DefaultMaterial)
{
    if (!a1->IsSetParam<USMMaterialListParam>()) {
        return DefaultMaterial;
    }

    assert(DefaultMaterial->IsSwitchable());

    auto v5 = *bit_cast<uint32_t *>(&DefaultMaterial->field_18);

    nglMaterialBase **v4 = nullptr;
    nglMaterialBase *v3 = nullptr;
    if (v5 < 4u &&
        (v4 = a1->Get<USMMaterialListParam>()->field_0,
         (v3 = v4[a1->Get<USMMaterialIndicesParam>()->field_0[v5]]) != nullptr) &&
        v3->m_shader == DefaultMaterial->m_shader) {
        return v3;
    }

    return DefaultMaterial;
}
