#include "base_entity_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "limited_timer.h"
#include "trace.h"
#include "vtbl.h"

VALIDATE_SIZE(base_entity_resource_handler, 0x10);

base_entity_resource_handler::base_entity_resource_handler() {}

bool base_entity_resource_handler::_handle(worldly_resource_handler::eBehavior behavior, limited_timer *timer)
{
    TRACE("base_entity_resource_handler::handle");

    if constexpr (1) {
        if (this->field_4.is_done() || behavior == LOAD) {
            return false;
        }

        if (!this->field_4.is_started()) {
            this->field_4.start();
            this->field_C = 0;
        }

        while (this->field_C < this->get_num_resources()) {
            if (this->handle_resource(behavior) || (timer != nullptr && timer->elapsed() >= timer->field_4)) {
                return true;
            }
        }

        this->post_handle_resources(behavior);

        this->field_4.done();

        return false;
    } else {
        return (bool)THISCALL(0x0055F850, this, behavior, 0, timer);
    }
}

void base_entity_resource_handler::post_handle_resources(worldly_resource_handler::eBehavior)
{
    ;
}

int base_entity_resource_handler::get_num_resources()
{
    int(__fastcall * func)(base_entity_resource_handler *) = CAST(func, get_vfunc(m_vtbl, 0x8));

    return func(this);
}

bool base_entity_resource_handler::handle_resource(eBehavior behavior)
{
    bool(__fastcall * func)(base_entity_resource_handler *, void *, eBehavior) = CAST(func, get_vfunc(m_vtbl, 0xC));

    return func(this, nullptr, behavior);
}
