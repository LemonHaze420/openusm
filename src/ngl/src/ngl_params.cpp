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

nglMaterialBase *select_mesh_material(
    nglParamSet<nglShaderParamSet_Pool> *a1,
    nglMaterialBase *DefaultMaterial)
{
    if (!a1->IsSetParam<USMMaterialListParam>()) {
        return DefaultMaterial;
    }

    assert(DefaultMaterial->IsSwitchable());
    const auto material_slot = *bit_cast<uint32_t *>(&DefaultMaterial->field_18);

    auto *material_list = a1->Get<USMMaterialListParam>()->field_0;
    auto *material_indices = a1->Get<USMMaterialIndicesParam>()->field_0;
    if (material_slot >= 4u || material_list == nullptr || material_indices == nullptr ||
        IsBadReadPtr(material_indices, 4)) {
        return DefaultMaterial;
    }

    const auto material_index = material_indices[material_slot];
    if (IsBadReadPtr(material_list,
                     sizeof(*material_list) * (static_cast<size_t>(material_index) + 1))) {
        return DefaultMaterial;
    }

    auto *material = material_list[material_index];
    if (material != nullptr && !IsBadReadPtr(material, sizeof(*material)) &&
        material->m_shader == DefaultMaterial->m_shader) {
        return material;
    }

    return DefaultMaterial;
}
