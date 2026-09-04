#include "scene_anim.h"

#include "comic_page_camera.h"
#include "common.h"
#include "func_wrapper.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(nalSceneAnimInstance, 0x1C);

void nalSceneAnimInstance::Render() const
{
    TRACE("nalSceneAnimInstance::Render");

    if (this->field_8 != nullptr) {
        sp_log("0x%08X", this->field_8->field_0->m_vtbl);
    }

    if constexpr (1) {
        auto v2 = this->field_14 / this->field_18->field_8->field_38;
        for (auto *it = this->field_8; it != nullptr; it = static_cast<decltype(it)>(it->field_C)) {
            if (it->field_8 == nullptr) {
                it->field_8 = static_cast<decltype(it->field_8)>(
                    it->field_0->m_vtbl->CreateInstance(it->field_0, nullptr, it->field_4));
            }

            it->field_0->m_vtbl->Render(it->field_0, nullptr, it->field_8, v2);
        }
    } else {
        THISCALL(0x0078D130, this);
    }
}

void scene_anim_patch()
{
    {
        FUNC_ADDRESS(address, &nalSceneAnimInstance::Render);
        REDIRECT(0x00741734, address);
        REDIRECT(0x007417F5, address);
        REDIRECT(0x00742086, address);
    }
}
