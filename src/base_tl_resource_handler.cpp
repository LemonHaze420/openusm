#include "base_tl_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "limited_timer.h"
#include "log.h"
#include "mashable_vector.h"
#include "resource_directory.h"
#include "resource_pack_slot.h"
#include "tlresource_location.h"
#include "trace.h"
#include "utility.h"
#include "worldly_pack_slot.h"
#include "variables.h"
#include "vtbl.h"

#include <vtbl.h>

VALIDATE_SIZE(base_tl_resource_handler, 0x14);

#if !STANDALONE_SYSTEM
limited_timer *&dword_95C824 = var<limited_timer *>(0x0095C824);

#else

limited_timer *&dword_95C824 = []() -> auto & {
    static limited_timer *g_dword_95C824{};
    return g_dword_95C824;
}();

#endif

void base_tl_resource_handler::_pre_handle_resources(worldly_resource_handler::eBehavior)
{
    return;
}

void base_tl_resource_handler::pre_handle_resources(worldly_resource_handler::eBehavior behavior)
{
    void(__fastcall * func)(void *, int, worldly_resource_handler::eBehavior) = CAST(func, get_vfunc(m_vtbl, 0x8));
    func(this, 0, behavior);
}

bool base_tl_resource_handler::handle_resource(worldly_resource_handler::eBehavior behavior, tlresource_location *loc)
{
    bool(__fastcall * func)(void *, int, worldly_resource_handler::eBehavior, tlresource_location *) =
        CAST(func, get_vfunc(m_vtbl, 0xC));
    return func(this, 0, behavior, loc);
}

bool base_tl_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("base_tl_resource_handler::handle");

    if constexpr (1) {
        if (this->field_4.is_done()) {
            return false;
        }

        if (!this->field_4.is_started()) {
            this->field_4.start();
            this->field_C = 0;

            this->pre_handle_resources(a2);
        }

        auto &res_dir = this->my_slot->get_resource_directory();
        const auto tlres_count = res_dir.get_tlresource_count(this->field_10);

        //sp_log("%d %d", field_C, tlres_count);
        while (this->field_C < tlres_count) {
            auto *loc = res_dir.get_tlresource_location(this->field_C, this->field_10);
            assert(loc != nullptr);

            dword_95C824 = a3;

            bool v9 = this->handle_resource(a2, loc);

            dword_95C824 = nullptr;
            if ((v9) || (a3 != nullptr && a3->elapsed() >= a3->field_4)) {
                return true;
            }
        }

        this->field_4.done();
        return false;
    } else {
        return (bool)THISCALL(0x00562EC0, this, a2, a3);
    }
}
