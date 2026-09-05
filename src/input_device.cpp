#include "input_device.h"

#include "common.h"
#include "vtbl.h"

VALIDATE_SIZE(input_device, 0x8);

input_device::input_device()
{
#if STANDALONE_SYSTEM
    m_vtbl = nullptr;
#else
    m_vtbl = CAST(m_vtbl, 0x0088E4D0);
#endif
    field_4 = INVALID_DEVICE_ID;
}

uint8_t input_device::normalize(int a1)
{
    return a1;
}

bool input_device::is_connected() const
{
    assert(m_vtbl != nullptr && m_vtbl->is_connected != nullptr);
    return m_vtbl->is_connected(this);
}

int input_device::get_axis_id(int axis)
{
    assert(m_vtbl != nullptr && m_vtbl->get_axis_id != nullptr);
    return m_vtbl->get_axis_id(this, nullptr, axis);
}

float input_device::get_axis_delta(int axis, int slot)
{
    assert(m_vtbl != nullptr && m_vtbl->get_axis_delta != nullptr);
    return m_vtbl->get_axis_delta(this, nullptr, axis, slot);
}

void input_device::poll()
{
    assert(m_vtbl != nullptr && m_vtbl->poll != nullptr);
    m_vtbl->poll(this);
}

float input_device::get_axis_old_state(int axis, int slot)
{
    assert(m_vtbl != nullptr && m_vtbl->get_axis_old_state != nullptr);
    return m_vtbl->get_axis_old_state(this, nullptr, axis, slot);
}

float input_device::get_axis_state(int axis, int slot)
{
    assert(m_vtbl != nullptr && m_vtbl->get_axis_state != nullptr);
    return m_vtbl->get_axis_state(this, nullptr, axis, slot);
}

device_id_t input_device::get_id() const
{
    assert(m_vtbl != nullptr && m_vtbl->get_id != nullptr);
    return m_vtbl->get_id(this);
}
