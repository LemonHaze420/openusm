#include "anim_event.h"

#include "common.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "memory.h"
#include "vtbl.h"

namespace {
void __fastcall destruct_anim_event(anim_event *self, void *)
{
    delete[] self->field_10;
    self->field_10 = nullptr;
    self->field_C.destruct_mashed_class();
    self->field_4.destruct_mashed_class();
}

void __fastcall unmash_anim_event(anim_event *self, void *, mash_info_struct *info, void *owner)
{
    using unmash_callback = void(__fastcall *)(event *, void *, mash_info_struct *, void *);
    auto *base = static_cast<std::intptr_t *>(event::native_vtable());
    reinterpret_cast<unmash_callback>(base[1])(self, nullptr, info, owner);
    info->unmash_class_in_place(self->field_C, self);
    if (self->field_10 != nullptr) {
        info->unmash_class(self->field_10, self);
    }
}

void *__fastcall delete_anim_event(anim_event *self, void *, unsigned int flags)
{
    self->~anim_event();
    if (flags & 1) {
        mem_dealloc(self, sizeof(*self));
    }
    return self;
}

int __fastcall anim_event_type(const anim_event *, void *)
{
    return 534;
}
bool __fastcall anim_event_parent(const anim_event *, void *, int type)
{
    return type == 539 || type == 573;
}
bool __fastcall anim_event_is_type(const anim_event *, void *, int type)
{
    return type == 534 || type == 539 || type == 573;
}
int __fastcall anim_event_size(const anim_event *, void *)
{
    return sizeof(anim_event);
}

void *native_anim_event_vtable()
{
    auto *base = static_cast<std::intptr_t *>(event::native_vtable());
    static std::intptr_t table[] = {
        reinterpret_cast<std::intptr_t>(&destruct_anim_event),
        reinterpret_cast<std::intptr_t>(&unmash_anim_event),
        reinterpret_cast<std::intptr_t>(&delete_anim_event),
        reinterpret_cast<std::intptr_t>(&anim_event_type),
        reinterpret_cast<std::intptr_t>(&anim_event_parent),
        reinterpret_cast<std::intptr_t>(&anim_event_is_type),
        base[6],
        reinterpret_cast<std::intptr_t>(&anim_event_size),
    };
    return table;
}
}  // namespace

VALIDATE_SIZE(anim_event, 0x18);

anim_event::anim_event(string_hash a2, string_hash a3, int count) : event(a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        this->m_vtbl = reinterpret_cast<std::intptr_t>(native_anim_event_vtable());
        this->field_C = a3;
        this->my_num_params = count;
        this->field_10 = count != 0 ? new string_hash[count] : nullptr;
    } else {
        THISCALL(0x004AD3F0, this, a2, a3, count);
    }
}

anim_event::~anim_event()
{
    if constexpr (STANDALONE_SYSTEM) {
        delete[] this->field_10;
    } else {
        THISCALL(0x0043A570, this);
    }
}
