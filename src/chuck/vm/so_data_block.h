#pragma once

#include <cstdint>

inline constexpr auto SO_DATA_BLOCK_FLAG_FROM_MASH = 1u;

struct generic_mash_header;
struct generic_mash_data_ptrs;

class so_data_block {
    int m_size;
    char *buffer;
    uint32_t flags;

public:
    so_data_block() = default;
    so_data_block(int a2);

    ~so_data_block();

    void operator=(int a2);

    void init(int a2);

    void destroy();

    void set_to_zero();

    void clear();

    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);

    auto size() const
    {
        return m_size;
    }

    bool is_from_mash() const
    {
        return (this->flags & SO_DATA_BLOCK_FLAG_FROM_MASH) != 0;
    }

    auto *get_buffer() const
    {
        return buffer;
    }

    bool is_address_in_buffer(int a2) const
    {
        return a2 >= (int)this->buffer && a2 < (int)&this->buffer[this->m_size];
    }
};
