#include "visual_item.h"

#include "common.h"
#include "conglom.h"
#include "entity_mash.h"
#include "memory.h"
#include "oldmath_po.h"
#include "vtbl.h"

#include <algorithm>
#include <array>
#include <cmath>

VALIDATE_SIZE(visual_item, 0xC8u);

namespace {
void *__fastcall visual_delete(visual_item *self, void *, unsigned flags)
{
    self->~visual_item();
    if (flags & 1)
        mem_dealloc(self, sizeof(visual_item));
    return self;
}
int __fastcall visual_size(visual_item *, void *)
{
    return sizeof(visual_item);
}
bool __fastcall visual_query(visual_item *, void *)
{
    return true;
}
void __fastcall visual_render(visual_item *self, void *, Float elapsed)
{
    self->_render(elapsed);
}
void __fastcall visual_attach(visual_item *self, void *, entity_base *owner, string_hash bone, float scale,
                              const vector3d *position, const vector3d *rotation, bool drawn)
{
    self->attach(owner, bone, scale, *position, *rotation, drawn);
}
int __fastcall visual_owner_flags(visual_item *self, void *)
{
    if (self->field_C4 && (self->field_C4->field_4 & 0x200)) {
        auto query =
            reinterpret_cast<int(__fastcall *)(entity_base *, void *)>(get_vfunc(self->field_C4->m_vtbl, 0x1E0));
        return query(self->field_C4, nullptr);
    }
    return 0;
}
}  // namespace

void *visual_item::native_vtable(void **actor_table)
{
    static std::array<void *, 0x29C / 4> table;
    std::copy_n(actor_table, 0x294 / 4, table.begin());
    table[0] = reinterpret_cast<void *>(&visual_delete);
    table[1] = reinterpret_cast<void *>(&visual_size);
    table[0xEC / 4] = reinterpret_cast<void *>(&visual_query);
    table[0x1AC / 4] = reinterpret_cast<void *>(&visual_render);
    table[0x294 / 4] = reinterpret_cast<void *>(&visual_attach);
    table[0x298 / 4] = reinterpret_cast<void *>(&visual_owner_flags);
    return table.data();
}

visual_item::visual_item(const string_hash &id, uint32_t flags) : actor(id, flags), field_C0(false), field_C4(nullptr)
{
#if STANDALONE_SYSTEM
    m_vtbl = ent_v_table_lookup[26];
#else
    m_vtbl = 0x00885098;
#endif
}

void visual_item::attach(entity_base *owner, string_hash bone, float scale, const vector3d &position,
                         const vector3d &rotation, bool)
{
    field_C4 = owner;
    entity_base *parent = owner;
    if (owner->get_flavor() == 12) {
        if (auto *member = static_cast<conglomerate *>(owner)->get_member(bone, true))
            parent = member;
    }
    set_parent(parent);

    po pose = po_identity_matrix;
    pose.set_position(position);
    const vector3d radians = rotation * 0.017453292f;
    for (int axis = 0; axis < 3; ++axis) {
        if (std::fabs(radians[axis]) <= 0.0001f)
            continue;
        po increment = po_identity_matrix;
        if (axis == 0)
            increment.set_rotate_x(Float{radians[axis]});
        else if (axis == 1)
            increment.set_rotate_y(Float{radians[axis]});
        else
            increment.set_rotate_z(Float{radians[axis]});
        pose.set_from_ptr_to_po_world(ptr_to_po{&pose.m, &increment.m});
        pose.set_position(position);
    }
    if (std::fabs(scale - 1.0f) > 0.0001f) {
        po increment = po_identity_matrix;
        increment.set_scale(vector3d{scale, scale, scale});
        pose.set_from_ptr_to_po_world(ptr_to_po{&pose.m, &increment.m});
        pose.set_position(position);
    }
    set_abs_po(pose);
    if (is_renderable())
        set_visible(true, false);
}

void visual_item::_render(Float fade)
{
    auto &pose = get_abs_po();
    auto x = pose.get_x_facing();
    auto y = pose.get_y_facing();
    auto z = pose.get_z_facing();
    const vector3d position = pose.get_position();
    const auto normalize = [](vector3d &axis) {
        const float squared = axis.x * axis.x + axis.y * axis.y + axis.z * axis.z;
        if (squared > 9.99999944e-11f)
            axis *= 1.0f / std::sqrt(squared);
    };
    normalize(x);
    normalize(y);
    normalize(z);
    pose.set_po(x, y, z, position);
    actor::_render(fade);
}
