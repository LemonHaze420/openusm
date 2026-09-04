#pragma once

#include "entity_handle_manager.h"

struct entity_base_vhandle {
    uint32_t field_0;

    entity_base_vhandle() = default;

    constexpr entity_base_vhandle(int p) : field_0(p) {}

    uint32_t get_goodies() const
    {
        return field_0;
    }

    bool operator<(const entity_base_vhandle &a1) const
    {
        return (this->field_0 < a1.field_0);
    }

    bool operator>(const entity_base_vhandle &a1) const
    {
        return (this->field_0 > a1.field_0);
    }

    bool operator==(const entity_base_vhandle &a1) const
    {
        return (this->field_0 == a1.field_0);
    }

    bool operator!=(const entity_base_vhandle &a1) const
    {
        return !(*this == a1);
    }

    //0x00601580
    entity_base *get_volatile_ptr() const;
};

inline constexpr entity_base_vhandle INVALID_HANDLE = {0};

inline constexpr auto INVALID_VHANDLE = 0;

template <typename T0, typename T1 = typename T0::base_type>
struct vhandle_type {
    using value_type = T0;

    entity_base_vhandle field_0;

    T0 *get_volatile_ptr() const
    {
        auto *result = static_cast<T0 *>(this->field_0.get_volatile_ptr());

        return result;
    }

    bool operator==(const vhandle_type<T0> &)
    {
        return get_volatile_ptr() != nullptr;
    }

    bool operator==(int)
    {
        return get_volatile_ptr() != nullptr;
    }

    bool operator==(const entity_base_vhandle &a1) const
    {
        return (this->field_0 == a1.field_0);
    }

    bool operator!=(const vhandle_type<T0> &arg)
    {
        return !(*this == arg);
    }
};
