#include "tentacle_interface.h"

#include "common.h"
#include "entity.h"
#include "event.h"
#include "event_manager.h"
#include "event_recipient_entry.h"
#include "func_wrapper.h"
#include "trace.h"

VALIDATE_SIZE(tentacle_interface, 0x38);

void tentacle_interface::initialize_polytubes()
{
    TRACE("tentacle_interface::initialize_polytubes");

    THISCALL(0x004E0460, this);
}

void zip_event_callback(event *the_event, entity_base_vhandle a2, void *params)
{
    assert(the_event != nullptr && params != nullptr);

    if ( a2.get_volatile_ptr() != nullptr ) {
        static_cast<tentacle_interface *>(params)->tentacle_zip_event_fired();
    }
}

void tentacle_interface::begin_zip(const vector3d &a2)
{
    this->field_10 = a2;
    auto v3 = this->field_34;
    this->field_28 = 0;
    if ( v3 == 0 )
    {
        auto v4 = this->field_4->my_handle;
        this->field_34 = event_manager::add_callback(
             event::ANIM_ACTION,
             v4,
             zip_event_callback,
             this,
             false);
    }
}

void tentacle_interface::tentacle_zip_event_fired()
{
    void (__fastcall *func)(void *) = CAST(func, 0x004CF6F0);
    func(this);
}
