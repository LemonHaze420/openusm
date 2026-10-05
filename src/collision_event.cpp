#include "collision_event.h"

#include "func_wrapper.h"
#include "common.h"
#include <array>

namespace {
int __fastcall collision_type(event *, void *) { return 538; }
int __fastcall collision_size(event *, void *) { return sizeof(collision_event); }
bool __fastcall collision_subclass(event *, void *, int type) { return type == 539 || type == 573; }
bool __fastcall collision_is_or_subclass(event *, void *, int type) { return type == 538 || type == 539 || type == 573; }
}
VALIDATE_SIZE(collision_event, 0x2C);

collision_event::collision_event(entity_base_vhandle arg0, const subdivision_node *a3, const vector3d &a4,
                                 const vector3d &a5)
    : event(collision_event::type_id)
{
    if constexpr (STANDALONE_SYSTEM) {
        static const auto table = [this] {
            std::array<std::intptr_t, 8> result{};
            const auto *base = reinterpret_cast<const std::intptr_t *>(m_vtbl);
            std::copy(base, base + result.size(), result.begin());
            result[3] = reinterpret_cast<std::intptr_t>(&collision_type);
            result[4] = reinterpret_cast<std::intptr_t>(&collision_subclass);
            result[5] = reinterpret_cast<std::intptr_t>(&collision_is_or_subclass);
            result[7] = reinterpret_cast<std::intptr_t>(&collision_size);
            return result;
        }();
        m_vtbl = reinterpret_cast<std::intptr_t>(table.data());
        other = arg0;
        obb = a3;
        position = a4;
        normal = a5;
    } else {
        THISCALL(0x005B1870, this, arg0, a3, &a4, &a5);
    }
}
