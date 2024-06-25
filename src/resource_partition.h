#pragma once

#include "resource_pack_streamer.h"

struct resource_pack_slot;
struct worldly_pack_slot;

enum resource_partition_enum {
    RESOURCE_PARTITION_START = 0,
    RESOURCE_PARTITION_HERO = 1,
    RESOURCE_PARTITION_LANG = 2,
    RESOURCE_PARTITION_MISSION = 3,
    RESOURCE_PARTITION_COMMON = 4,
    RESOURCE_PARTITION_STRIP = 5,
    RESOURCE_PARTITION_DISTRICT = 6,
    RESOURCE_PARTITION_STANDALONE = 7,
    RESOURCE_PARTITION_END = 8,
};

struct resource_partition {
    int field_0;
    resource_partition_enum field_4;

    _std::vector<resource_pack_slot *> m_pack_slots;

    resource_pack_streamer streamer;

private:
    uint8_t *m_partition_buffer;
    int partition_buffer_used;
    int partition_buffer_size;

public:
    //0x005427C0
    resource_partition(resource_partition_enum a2);

    //0x0053DFD0
    ~resource_partition();

    void * operator new(size_t size);

    void operator delete(void *ptr, size_t size);

    auto get_type() const {
        return field_4;
    }

    void set_buffer(uint8_t *buffer) {
        this->m_partition_buffer = buffer;
    }

    uint8_t * get_buffer() {
        return this->m_partition_buffer;
    }

    void set_buffer_used(int used) {
        this->partition_buffer_used = used;
    }

    auto get_buffer_used() const {
        return this->partition_buffer_used;
    }

    void set_buffer_size(int size) {
        this->partition_buffer_size = size;
    }

    auto get_buffer_size() const {
        return this->partition_buffer_size;
    }

    //0x00556090
    void frame_advance(Float a1, limited_timer *a2);

    //0x0050E010
    bool has_room_for_slot(int a2);

    //0x00537B30
    void clear();

    //0x00542840
    void push_pack_slot(int memory_amount_to_reserve, void *a3);

    //0x00537BB0
    void pop_pack_slot();

    resource_pack_streamer *get_streamer() {
        return (&streamer);
    }

    auto &get_pack_slots() {
        return this->m_pack_slots;
    }
};

extern void resource_partition_patch();
