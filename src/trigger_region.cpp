#include "trigger_region.h"

#include "actor.h"
#include "common.h"
#include "memory.h"
#include "oldmath_po.h"
#include "vtbl.h"
#include "wds.h"
#include "wds_entity_manager.h"
#include "mash_info_struct.h"

#include <array>

VALIDATE_SIZE(trigger_region, 0x4);
VALIDATE_SIZE(box_region, 0x20);
VALIDATE_SIZE(named_trigger_box_region, 0x8);
VALIDATE_SIZE(point_dist_region, 0x18);

namespace {
template <typename T, uint32_t Type>
uint32_t __fastcall region_type(const T *)
{
    return Type;
}

template <typename T>
int __fastcall region_size(const T *)
{
    return sizeof(T);
}

template <typename T>
bool __fastcall region_contains(const T *self, int, const vector3d *position, actor *owner)
{
    return self->contains(position, owner);
}

bool __fastcall region_subclass(const trigger_region *, int, mash::virtual_types_enum type)
{
    return type == 569 || type == 573;
}

bool __fastcall region_is_or_subclass(const trigger_region *self, int, mash::virtual_types_enum type)
{
    return self->get_virtual_type_enum() == static_cast<uint32_t>(type) ||
        region_subclass(self, 0, type);
}

void __fastcall named_region_unmash(named_trigger_box_region *self, int, mash_info_struct *info, void *context)
{
    self->_unmash(info, context);
}

template <typename T, uint32_t Type>
void *region_vtable()
{
    static auto table = [] {
        std::array<void *, 9> result {};
        result[3] = bit_cast<void *>(&region_type<T, Type>);
        result[4] = bit_cast<void *>(&region_subclass);
        result[5] = bit_cast<void *>(&region_is_or_subclass);
        result[6] = bit_cast<void *>(&region_contains<T>);
        result[8] = bit_cast<void *>(&region_size<T>);
        if constexpr (Type == 548) {
            result[1] = bit_cast<void *>(&named_region_unmash);
        }
        return result;
    }();
    return table.data();
}
}

void *box_region::native_vtable()
{
    return region_vtable<box_region, 545>();
}

void *named_trigger_box_region::native_vtable()
{
    return region_vtable<named_trigger_box_region, 548>();
}

void *point_dist_region::native_vtable()
{
    return region_vtable<point_dist_region, 549>();
}

box_region::box_region(from_mash_in_place_constructor *tag) : upper(tag), lower(tag)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x0087553C;
}

named_trigger_box_region::named_trigger_box_region(from_mash_in_place_constructor *tag) : name(tag)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x008754F4;
}

point_dist_region::point_dist_region(from_mash_in_place_constructor *tag) : center(tag)
{
    m_vtbl = STANDALONE_SYSTEM ? bit_cast<std::intptr_t>(native_vtable()) : 0x00875518;
}

void trigger_region::unmash(mash_info_struct *info, void *context)
{
    const auto type = get_virtual_type_enum();
    if (type == 548) {
        static_cast<named_trigger_box_region *>(this)->_unmash(info, context);
    } else {

        assert(type == 545 || type == 549);
    }
}

void named_trigger_box_region::_unmash(mash_info_struct *info, void *)
{
    info->unmash_class_in_place(name, this);
}

int trigger_region::get_mash_sizeof() const
{
    auto fn = bit_cast<int(__fastcall *)(const trigger_region *)>(get_vfunc(m_vtbl, 0x20));
    return fn(this);
}

bool trigger_region::is_inside_trigger_region(const vector3d *position, actor *owner) const
{
    auto fn = bit_cast<bool(__fastcall *)(const trigger_region *, int, const vector3d *, actor *)>(
        get_vfunc(m_vtbl, 0x18));
    return fn(this, 0, position, owner);
}

bool box_region::contains(const vector3d *position, actor *owner) const
{
    const auto local = actor_relative ? owner->get_abs_po().inverse_xform(*position) : *position;
    return lower.x < local.x && local.x < upper.x &&
        lower.y < local.y && local.y < upper.y &&
        lower.z < local.z && local.z < upper.z;
}

bool point_dist_region::contains(const vector3d *position, actor *owner) const
{
    const auto absolute_center = actor_relative ? owner->get_abs_po().slow_xform(center) : center;
    const double dx = static_cast<double>(absolute_center.x) - position->x;
    const double dy = static_cast<double>(absolute_center.y) - position->y;
    const double dz = static_cast<double>(absolute_center.z) - position->z;
    const double squared_distance = dx * dx + dy * dy + dz * dz;
    return !(squared_distance > radius_squared);
}

bool named_trigger_box_region::contains(const vector3d *position, actor *) const
{
    auto *target = g_world_ptr->ent_mgr.get_entity(name);
    if (target == nullptr) {
        return false;
    }
    auto fn = bit_cast<bool(__fastcall *)(entity_base *, int, const vector3d *)>(
        get_vfunc(target->m_vtbl, 0x16C));
    return fn(target, 0, position);
}

void trigger_region::destruct_mashed_class()
{
    if (get_virtual_type_enum() == 548) {
        static_cast<named_trigger_box_region *>(this)->name.destruct_mashed_class();
    }
}

void trigger_region::delete_owned()
{
    const auto size = get_mash_sizeof();
    destruct_mashed_class();
    mem_dealloc(this, size);
}
