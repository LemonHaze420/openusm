#include "resource_pack_slot.h"

#include "common.h"
#include "func_wrapper.h"
#include "limited_timer.h"
#include "os_developer_options.h"
#include "debugutil.h"
#include "utility.h"
#include "vtbl.h"
#include "variables.h"

#include "tlresource_directory.h"
#include "trace.h"

#include "resource_directory.h"
#include "resource_manager.h"
#include "resource_pack_streamer.h"
#include "worldly_pack_slot.h"

#include <cassert>

VALIDATE_OFFSET(resource_pack_slot, field_78, 0x78);
VALIDATE_OFFSET(resource_pack_slot, m_callback, 0x8C);

VALIDATE_SIZE(resource_pack_slot, 0x94);

Var<resource_pack_slot *> resource_pack_slot::current_alloc_slot{0x0095C820};

resource_pack_slot::resource_pack_slot() {
    if constexpr (1) {
        this->pack_directory = {};

        this->field_78 = {};

        this->field_84 = 0;
        this->m_slot_state = SLOT_STATE_EMPTY;
        this->slot_size = 0;
        this->header_mem_addr = nullptr;
        this->m_callback = nullptr;
        this->clear_pack();
    } else {
        THISCALL(0x00531C70, this);
    }
}

resource_pack_slot::~resource_pack_slot() {
    if constexpr (1) {
        clear_slot();

    } else {
        THISCALL(0x00531CF0, this);
    }
}

bool resource_pack_slot::is_data_ready() 
{
    return (this->m_slot_state == 2
            || this->m_slot_state == 3
            || this->m_slot_state == 4);
}

void resource_pack_slot::notify_unload_started()
{
    assert(m_slot_state == SLOT_STATE_READY);

    auto client_done = this->try_callback(CALLBACK_PRE_DESTRUCT, nullptr);
    //assert(client_done);

    this->m_slot_state = static_cast<decltype(m_slot_state)>(3);
}

uint8_t *resource_pack_slot::get_resource(const resource_key &resource_id,
                                          int *a3,
                                          resource_pack_slot **a4)
{
    TRACE("resource_pack_slot::get_resource", resource_id.get_platform_string(g_platform()).c_str());

    auto &res_dir = this->get_resource_directory();
    return res_dir.get_resource(resource_id, a3, a4);
}

void resource_pack_slot::clear_slot() {
    this->slot_size = 0;
    this->header_mem_addr = nullptr;
    this->m_callback = nullptr;

    this->clear_pack();
}

void resource_pack_slot::clear_pack() {
    if constexpr (0)
    {
        assert(m_slot_state == SLOT_STATE_EMPTY);

        this->field_4 = {};

        this->pack_size = 0;

        this->pack_directory.clear();

        this->field_78 = {};
        this->field_80.clear();
    }
    else
    {
        void (__fastcall *func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x10));
        func(this);
    }
}

void resource_pack_slot::set_memory_area(uint8_t *starting_addr, uint32_t how_many_bytes, bool a4)
{
    assert(is_empty());
    assert(header_mem_addr == nullptr);
    assert(slot_size == 0);
    assert(pack_size == 0);
    assert(starting_addr != nullptr);
    assert(how_many_bytes > 0);

    this->header_mem_addr = starting_addr;
    this->slot_size = how_many_bytes;
    this->field_90 = a4;
    std::memset(this->header_mem_addr, 0x77, how_many_bytes);
}

resource_directory &resource_pack_slot::get_resource_directory() {
    TRACE("resource_pack_slot::get_resource_directory");
    assert(is_data_ready());
    assert(pack_directory.get_resource_directory() != nullptr);

    return (*this->pack_directory.get_resource_directory());
}

void resource_pack_slot::set_resource_directory(resource_directory *directory) {
    assert(m_slot_state == SLOT_STATE_STREAMING);

    this->pack_directory.set_resource_directory(directory);
}

void resource_pack_slot::notify_load_started(const resource_key &a2,
                                             uint32_t pack_size,
                                             bool (*cb)(resource_pack_slot::callback_enum,
                                                        resource_pack_streamer *,
                                                        resource_pack_slot *,
                                                        limited_timer *),
                                             const resource_pack_token &token)
{
    TRACE("resource_pack_slot::notify_load_started");

    if constexpr (0)
    {
        assert(m_slot_state == SLOT_STATE_EMPTY);

        this->clear_pack();

        this->field_4 = a2;

        this->pack_size = pack_size;
        this->m_callback = cb;
        this->field_78 = token;
        this->m_slot_state = SLOT_STATE_STREAMING;
        if (cb != nullptr) {
            cb((callback_enum) 0, &this->field_88->streamer, this, nullptr);
        }
    } else {
        THISCALL(0x0050E240, this, &a2, pack_size, cb, &token);
    }
}

void resource_pack_slot::notify_load_finished() {
    assert(m_slot_state == SLOT_STATE_STREAMING);

    this->m_slot_state = static_cast<slot_state_t>(2);

    [[maybe_unused]] auto client_done = this->try_callback((callback_enum) 1, nullptr);
    //assert(client_done);
}

bool resource_pack_slot::is_empty() {
    return m_slot_state == SLOT_STATE_EMPTY;
}

bool resource_pack_slot::try_callback(callback_enum a2, limited_timer *a3) {
    if constexpr (1)
    {
        auto *cb = this->m_callback;
        if (cb == nullptr) {
            return false;
        }

        bool result = cb(a2, &this->field_88->streamer, this, a3);
        return result;

    } else {
        return (bool) THISCALL(0x0050E210, this, a2, a3);
    }
}

bool resource_pack_slot::on_load(limited_timer *a1) {
    if constexpr (0)
    {
        return (a1 != nullptr && a1->elapsed() >= a1->field_4);
    }
    else
    {
        bool (__fastcall *func)(resource_pack_slot *, int, limited_timer *) =
            CAST(func, get_vfunc(m_vtbl, 0x0));
        return func(this, 0, a1);
    }
}

bool resource_pack_slot::on_unload([[maybe_unused]] limited_timer *a2)
{
    if constexpr (0)
    {
        if (!this->field_80.is_done()) {
            if (!this->field_80.is_started()) {
                this->field_80.start();
            }

            if (!this->field_80.is_done()) {
                auto &dir = this->get_resource_directory();
                dir.release_mem();
                this->field_80.done();
            }
        }

        return false;
    }
    else
    {
        bool (__fastcall *func)(void *, int, limited_timer *) = 
            CAST(func, get_vfunc(m_vtbl, 0x4));
        return func(this, 0, a2);
    }
}

bool resource_pack_slot::slot_allocators_empty() {
    auto *v1 = &this->pack_directory;
    auto *v2 = v1->get_resource_directory();
    return v2->sub_9C2EE0();
}

void resource_pack_slot::frame_advance([[maybe_unused]] Float a2, limited_timer *a3) {

    TRACE("resource_pack_slot::frame_advance", int(this->m_slot_state));

    if constexpr (0)
    {
        switch (this->m_slot_state)
        {
        case SLOT_STATE_EMPTY:
        case SLOT_STATE_STREAMING:
        case SLOT_STATE_READY:
            break;

        case 2: {
            auto *resource_context = resource_manager::get_best_context(this);
            auto *__old_context = resource_manager::push_resource_context(resource_context);

            {
                if (!this->on_load(a3) && !this->try_callback(CALLBACK_CONSTRUCT, a3))
                {
                    assert(this->slot_allocators_empty());

                    this->m_slot_state = SLOT_STATE_READY;
                }
            }

            resource_manager::pop_resource_context();
            assert(resource_manager::get_resource_context() == __old_context);

            break;
        }
        case 3: {
            auto *__old_context = resource_manager::push_resource_context(this);

            {
                if (!this->try_callback(CALLBACK_DESTRUCT, a3) && !this->on_unload(a3))
                {
                    this->m_slot_state = SLOT_STATE_EMPTY;
                }
            }

            resource_manager::pop_resource_context();
            assert(resource_manager::get_resource_context() == __old_context);

        } break;
        default: {
            sp_log("Unknown slot state 0x%x", this->m_slot_state);
            assert(0);
        } break;
        }

    } else {
        THISCALL(0x005429F0, this, a2, a3);
    }
}

void *resource_pack_slot::slot_allocate(unsigned int a1, unsigned int a2)
{
    assert(current_alloc_slot() != nullptr);

    auto res_dir = resource_pack_slot::current_alloc_slot()->pack_directory.get_resource_directory();

    auto *p = res_dir->allocate_from_pool(a1, a2);
    assert(p > current_alloc_slot()->header_mem_addr);
    assert(p < current_alloc_slot()->header_mem_addr + current_alloc_slot()->slot_size);

    return p;
}

void resource_pack_slot_patch() {
    {
        FUNC_ADDRESS(address, &resource_pack_slot::notify_load_started);
        //REDIRECT(0x0054C7B4, address);
    }

    {
        FUNC_ADDRESS(address, &resource_pack_slot::frame_advance);
        REDIRECT(0x00551176, address);
    }

}
