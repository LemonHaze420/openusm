#include "resource_partition.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "trace.h"
#include "resource_pack_slot.h"
#include "utility.h"
#include "vtbl.h"
#include "worldly_pack_slot.h"

#include <cassert>

VALIDATE_SIZE(resource_partition, 0xB4u);

resource_partition::resource_partition(resource_partition_enum a2)
{
    if constexpr (1)
    {
        this->field_4 = a2;
        this->clear();

        this->streamer.init(this, &this->m_pack_slots);
    }
    else
    {
        THISCALL(0x005427C0, this, a2);
    }
}

resource_partition::~resource_partition()
{
    if constexpr (1)
    {
        this->clear();
        this->m_pack_slots.clear();
    }
    else
    {
        THISCALL(0x0053DFD0, this);
    }

}

void * resource_partition::operator new(size_t size) {
    return mem_alloc(size);
}

void resource_partition::operator delete(void *ptr, size_t size) {
    mem_dealloc(ptr, size);
}


void resource_partition::frame_advance(Float a1, limited_timer *a2) {
    this->streamer.frame_advance({a1}, a2);
}

bool resource_partition::has_room_for_slot(int a2)
{
    auto v2 = a2;
    if (a2 % 4096 > 0) {
        v2 = (a2 / 4096 + 1) << 12;
    }

    return v2 + this->partition_buffer_used <= this->partition_buffer_size;
}

void resource_partition::clear()
{
    TRACE("resource_partition::clear");

    assert(streamer.is_idle());

    if constexpr (1)
    {
        for (uint32_t i = 0; i < this->m_pack_slots.size(); ++i)
        {
            auto *slot = this->m_pack_slots[i];

            worldly_pack_slot *delete_me = CAST(delete_me, slot);
            assert(delete_me != nullptr);

            assert(delete_me->is_empty());

            if (delete_me != nullptr) {
                delete_me->m_vtbl->finalize(delete_me, nullptr, true);
            }
        }

        if (this->m_pack_slots.m_first != nullptr) {
            delete this->m_pack_slots.m_first;
        }

        this->m_pack_slots = {};

        this->streamer.clear();
        this->field_0 = 0;
        this->field_A8 = nullptr;
        this->partition_buffer_used = 0;
        this->partition_buffer_size = 0;
    }
    else
    {
        THISCALL(0x00537B30, this);
    }
}

void resource_partition::pop_pack_slot()
{
    if constexpr (1)
    {
        assert(m_pack_slots.size() > 0);

        assert(m_pack_slots.back() != nullptr && m_pack_slots.back()->is_empty());

        auto *slot = m_pack_slots.back();
        if (slot->field_90) {
            this->partition_buffer_used -= slot->get_slot_size();
        }

        delete slot;

        if (!this->m_pack_slots.empty()) {
#ifndef TEST_CASE
            --this->m_pack_slots.m_last;
#else
            this->m_pack_slots.resize(this->m_pack_slots.size() - 1);
#endif
        }

    } else {
        THISCALL(0x00537BB0, this);
    }
}

void resource_partition::push_pack_slot(int memory_amount_to_reserve, void *a3)
{
    TRACE("resource_partition::push_pack_slot");

    if constexpr (0)
    {
        assert(memory_amount_to_reserve > 0);

        auto reserve_size = memory_amount_to_reserve;
        if (memory_amount_to_reserve % 4096 > 0) {
            reserve_size = (reserve_size / 4096 + 1) << 12;
        }

        auto *starting_addr = &this->field_A8[this->partition_buffer_used];
        if (a3 != nullptr) {
            starting_addr = static_cast<uint8_t *>(a3);
        } else {
            assert((partition_buffer_used + reserve_size <= partition_buffer_size) &&
                   "Make sure we have enough room.");

            starting_addr = &this->field_A8[this->partition_buffer_used];
            this->partition_buffer_used += reserve_size;
        }

        auto *mem = mem_alloc(sizeof(worldly_pack_slot));
        auto *slot = new (mem) worldly_pack_slot{};

        slot->set_memory_area(starting_addr, reserve_size, a3 == nullptr);
        slot->set_partition(this);

        //push_back
        {
            auto *pack_slots = &this->m_pack_slots;

            auto size = pack_slots->size();
            auto capacity = pack_slots->capacity();

            if (size < capacity) {
                auto *v13 = pack_slots->m_last;
                *v13 = slot;
                pack_slots->m_last = v13 + 1;
            } else {
                void (__fastcall *_Insert_n)(void *, void *, void *, int, worldly_pack_slot **) = CAST(_Insert_n, 0x0056A260);

                _Insert_n(pack_slots, nullptr, pack_slots->m_last, 1, &slot);
            }
        }
    }
    else
    {
        THISCALL(0x00542840, this, memory_amount_to_reserve, a3);
    }
}

void resource_partition_patch()
{
    {
        FUNC_ADDRESS(address, &resource_partition::push_pack_slot);
        REDIRECT(0x00558C18, address);
        REDIRECT(0x005D208F, address);
        REDIRECT(0x005D81A8, address);
    }

    {
        FUNC_ADDRESS(address, &resource_partition::clear);
        //REDIRECT(0x00558A41, address);
    }
}
