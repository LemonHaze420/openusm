#include "als_use_anim_only.h"

#include "common.h"
#include "vtbl.h"
#include "actor.h"
#include "als_animation_logic_system.h"
#include "animation_controller.h"
#include "oldmath_po.h"
#include <array>
#include <algorithm>

namespace als {

VALIDATE_SIZE(use_anim_only, 0x14);

void use_anim_only::post_anim_action(Float arg0)
{
    if constexpr (STANDALONE_SYSTEM) {
        using scalar_fn = double(__fastcall *)(use_anim_only *, void *);
        using speed_fn = void(__fastcall *)(use_anim_only *, void *, Float);
        const Float speed = reinterpret_cast<scalar_fn>(get_vfunc(m_vtbl, 0x48))(this, nullptr);
        reinterpret_cast<speed_fn>(get_vfunc(m_vtbl, 0x40))(this, nullptr, speed);
        const auto previous_position = the_actor->get_abs_position();
        po offset{};
        field_4->get_animation_controller()->get_curr_po_offset(offset);
        offset.set_from_ptr_to_po_world(ptr_to_po{&offset.m, &the_actor->get_rel_po().m});
        offset.sub_48D840();
        the_actor->get_rel_po() = offset;
        the_actor->dirty_family(false);
        if (the_actor->is_ext_flagged(0x8004u))
            the_actor->dirty_model_po_family();
        the_actor->po_changed();
        const auto translation = the_actor->get_abs_position() - previous_position;
        the_actor->set_frame_delta_trans(translation, arg0);
    } else {
        void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x24));
        func(this, nullptr, arg0);
    }
}

namespace {
unsigned __fastcall anim_only_type(use_anim_only *, void *)
{
    return 525;
}
void __fastcall anim_only_post(use_anim_only *self, void *, Float elapsed)
{
    self->post_anim_action(elapsed);
}
}  // namespace

void *use_anim_only::native_vtable()
{
    static auto table = [] {
        std::array<void *, 20> result;
        auto *base = static_cast<void **>(motion_compensator::native_vtable(490));
        std::copy_n(base, result.size(), result.begin());
        result[0x0C / 4] = reinterpret_cast<void *>(&anim_only_type);
        result[0x24 / 4] = reinterpret_cast<void *>(&anim_only_post);
        return result;
    }();
    return table.data();
}

}  // namespace als
