#include "base_engine_resource_handler.h"

#include "func_wrapper.h"
#include "limited_timer.h"
#include "resource_directory.h"
#include "trace.h"
#include "vtbl.h"
#include "worldly_pack_slot.h"

base_engine_resource_handler::base_engine_resource_handler() {}

bool base_engine_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("base_engine_resource_handler::handle");

    if constexpr (1) {
        if (this->field_4.is_done()) {
            return false;
        }

        if (!this->field_4.is_started()) {
            auto *v6 = this->my_slot;

            this->field_4.start();

            this->field_C = v6->get_resource_directory().get_type_start_idxs(this->field_10);
            this->pre_handle_resources(a2);
        }

        resource_location v10{};

        auto &directory = this->my_slot->get_resource_directory();
        auto v9 = directory.get_type_start_idxs(this->field_10) + directory.get_resource_count(this->field_10);

        while (this->field_C < v9) {
            auto idx = this->field_C;
            auto *loc = directory.get_resource_location(idx);

            assert(loc != nullptr);

            if (this->handle_resource(a2, loc) || (a3 != nullptr && (a3->elapsed() >= a3->field_4))) {
                return true;
            }
        }

        this->field_4.done();

        return false;

    } else {
        return (bool)THISCALL(0x00562DF0, this, a2, a3);
    }
}

void base_engine_resource_handler::_pre_handle_resources(worldly_resource_handler::eBehavior)
{
    ;
}

void base_engine_resource_handler::pre_handle_resources(worldly_resource_handler::eBehavior behavior)
{
    void(__fastcall * func)(void *, int, worldly_resource_handler::eBehavior) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this, 0, behavior);
}

bool base_engine_resource_handler::handle_resource(worldly_resource_handler::eBehavior a2, resource_location *a3)
{
    bool(__fastcall * func)(void *, int, worldly_resource_handler::eBehavior, resource_location *) =
        CAST(func, get_vfunc(m_vtbl, 0xC));

    return func(this, 0, a2, a3);
}
