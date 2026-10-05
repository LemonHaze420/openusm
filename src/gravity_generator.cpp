#include "gravity_generator.h"

#include "actor.h"
#include "common.h"
#include "os_developer_options.h"
#include "physical_interface.h"
#include "time_interface.h"
#include "wds.h"

VALIDATE_SIZE(gravity_generator, 0x8);

namespace {
gravity_generator *__fastcall destroy_gravity(gravity_generator *self, void *, unsigned char flags)
{
    self->~gravity_generator();
    if ((flags & 1) != 0)
        ::operator delete(self);
    return self;
}
bool __fastcall gravity_active(gravity_generator *self, void *)
{
    return self->field_4;
}
void __fastcall set_gravity_active(gravity_generator *self, void *, bool active)
{
    self->field_4 = active;
}
void __fastcall advance_gravity(gravity_generator *self, void *, Float elapsed)
{
    self->frame_advance(elapsed);
}
struct gravity_vtable {
    decltype(&destroy_gravity) destroy;
    decltype(&gravity_active) active;
    decltype(&set_gravity_active) set_active;
    decltype(&advance_gravity) advance;
};
const gravity_vtable native_gravity_table{destroy_gravity, gravity_active, set_gravity_active, advance_gravity};
}  // namespace

gravity_generator::gravity_generator()
{
    field_4 = true;
    field_5 = false;
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<std::intptr_t>(&native_gravity_table);
#else
    m_vtbl = 0x0088817C;
#endif
}

void gravity_generator::frame_advance(Float elapsed)
{
    if (!os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(23)) ||
        physical_interface::all_phys_interfaces == nullptr)
        return;
    for (auto *physical : *physical_interface::all_phys_interfaces) {
        auto *owner = physical->field_4;
        if ((physical->field_C & 5u) != 5u || physical->field_A4 > 0.0f || (owner->field_4 & 0x40u) == 0 ||
            owner->is_in_limbo() || (physical->field_C & 0x88000u) != 0 || physical->field_174 != nullptr ||
            physical->is_effectively_standing())
            continue;
        const float scale = owner->field_58 != nullptr ? static_cast<float>(owner->field_58->sub_4ADE50())
                                                       : g_world_ptr->time_manager.field_0;
        const float gravity_multiplier =
            physical->field_D8 <= 0.0f ? physical->m_gravity_multiplier : physical->field_D4;
        auto impulse = physical->field_74 * g_gravity;
        impulse *= physical->field_10;
        impulse *= scale * elapsed.value * physical->field_1AC;
        impulse *= gravity_multiplier;
        physical->apply_force_increment(impulse, static_cast<physical_interface::force_type>(0), ZEROVEC, 0);
    }
}
