#pragma once

#include "resource_key.h"
#include "resource_pack_directory.h"
#include "resource_pack_token.h"
#include "timed_progress.h"

#include "float.hpp"

struct limited_timer;
struct resource_pack_streamer;
struct resource_directory;
struct resource_pack_header;
struct resource_partition;

enum slot_state_t {
    SLOT_STATE_EMPTY = 0,
    SLOT_STATE_STREAMING = 1,
    SLOT_STATE_LOADING = 2,
    SLOT_STATE_UNLOADING = 3,
    SLOT_STATE_READY = 4,
};

struct resource_pack_slot {

    enum callback_enum {
		CALLBACK_LOAD_STARTED = 0,
        CALLBACK_CONSTRUCT = 3,
        CALLBACK_PRE_DESTRUCT = 4,
        CALLBACK_DESTRUCT = 5,
    };

    struct {
        bool (__fastcall *on_load)(resource_pack_slot *, void *, limited_timer *);
        bool (__fastcall *on_unload)(resource_pack_slot *, void *, limited_timer *);
        resource_pack_slot * (__fastcall *finalize)(resource_pack_slot *, void *, bool);
        void (__fastcall *clear_slot)(resource_pack_slot *);
        void (__fastcall *clear_pack)(resource_pack_slot *);
    } *m_vtbl;

private:
    resource_key field_4;
    slot_state_t m_slot_state;
    int slot_size;
    int pack_size;
    uint8_t *header_mem_addr;

    resource_pack_directory pack_directory;
    resource_pack_token field_78;
    timed_progress field_80;
    resource_partition *field_88;

    bool (*m_callback)(callback_enum,
                       resource_pack_streamer *,
                       resource_pack_slot *,
                       limited_timer *);

public:
    bool field_90;

public:

    //0x00531C70
    resource_pack_slot();

    //0x00531CF0
    //virtual
    ~resource_pack_slot();

    void * operator new(std::size_t sz);

    void operator delete(void *ptr, std::size_t sz);

    resource_partition * get_partition() {
        return field_88;
    }

    void set_partition(resource_partition *p) {
        field_88 = p;
    }

    auto &get_name_key() const {
        return field_4;
    }

	resource_pack_token &get_pack_token();

    auto *get_header_mem_addr()
    {
        return header_mem_addr;
    }

    bool is_empty() const {
        return m_slot_state == SLOT_STATE_EMPTY;
    }

    bool is_pack_loading() const {
        return (this->m_slot_state == SLOT_STATE_LOADING);
    }

    bool is_pack_unloading() const {
        return (this->m_slot_state == SLOT_STATE_UNLOADING);
    }

    bool is_pack_ready() const {
        return (m_slot_state == SLOT_STATE_READY);
    }

    auto get_slot_size() const {
        return slot_size;
    }

    auto get_pack_size() const {
        return pack_size;
    }

    bool is_data_ready() const
    {
        return (this->m_slot_state == SLOT_STATE_LOADING
                || this->m_slot_state == SLOT_STATE_UNLOADING
                || this->m_slot_state == SLOT_STATE_READY);
    }

    resource_pack_directory & get_resource_pack_directory();

    resource_directory &get_resource_directory();

    void set_resource_directory(resource_directory *directory);

    void notify_load_cancelled();

    void notify_unload_started();

    //0x0050E240
    void notify_load_started(const resource_key &a2,
                             uint32_t a3,
                             bool (*cb)(resource_pack_slot::callback_enum,
                                        resource_pack_streamer *,
                                        resource_pack_slot *,
                                        limited_timer *),
                             const resource_pack_token &a5);

    void notify_load_finished();

    void set_memory_area(uint8_t *starting_addr, uint32_t how_many_bytes, bool a4);

    //0x0050E210
    bool try_callback(callback_enum a2, limited_timer *a3);

    uint8_t *get_resource(const resource_key &resource_id, int *a3, resource_pack_slot **a4);

    bool slot_allocators_empty();

    //0x005429F0
    void frame_advance(Float a2, limited_timer *a3);

    //0x0052A980
    static void *slot_allocate(unsigned int a1, unsigned int a2);

    static resource_pack_slot * & current_alloc_slot;

    //0x0050E1B0
    //virtual
    bool on_load(limited_timer *a1);

    //0x0052A910
    //virtual
    bool on_unload(limited_timer *a2);

    //0x0050E170
    //virtual
    void clear_slot();

    //0x0052A8B0
    //virtual
    void clear_pack();
};

struct ptr_to_resource_pack_slot {
    resource_pack_slot *field_0;
};

extern void resource_pack_slot_patch();
