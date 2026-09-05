#include "keyboard_device.h"

#include "console.h"
#include "game.h"
#include "input.h"
#include "keyboard.h"
#include "trace.h"
#include "utility.h"

#include <optional>

#if STANDALONE_SYSTEM
namespace {
device_id_t __fastcall keyboard_get_id(const input_device *device)
{
    return static_cast<device_id_t>(device->field_4);
}

int __fastcall keyboard_get_axis_count(const input_device *)
{
    return KB_NUM_AXES;
}

int __fastcall keyboard_get_axis_id(input_device *, void *, int axis)
{
    return axis;
}

float __fastcall keyboard_get_axis_state(input_device *device, void *, int axis, int slot)
{
    return static_cast<keyboard_device *>(device)->_get_axis_state(axis, slot);
}

float __fastcall keyboard_get_axis_old_state(input_device *device, void *, int axis, int slot)
{
    return static_cast<keyboard_device *>(device)->_get_axis_old_state(axis, slot);
}

float __fastcall keyboard_get_axis_delta(input_device *device, void *, int axis, int slot)
{
    return static_cast<keyboard_device *>(device)->_get_axis_delta(axis, slot);
}

void __fastcall keyboard_poll(input_device *device)
{
    static_cast<keyboard_device *>(device)->_poll();
}

void __fastcall keyboard_finalize(input_device *, void *, bool)
{
}

bool __fastcall keyboard_is_connected(const input_device *device)
{
    return static_cast<const keyboard_device *>(device)->_is_connected();
}

int __fastcall keyboard_clear_state(input_device *device)
{
    static_cast<keyboard_device *>(device)->_clear_state();
    return 0;
}

void __fastcall keyboard_vibrate(input_device *, void *, int, int, int, int)
{
}

void __fastcall keyboard_vibrate_scalar(input_device *, void *, Float)
{
}

void __fastcall keyboard_stop_vibration(input_device *)
{
}

bool __fastcall keyboard_is_vibrator_present(const input_device *device)
{
    return static_cast<const keyboard_device *>(device)->_is_vibrator_present();
}
}
#endif

keyboard_device::keyboard_device()
{
    using vtbl_t = std::decay_t<decltype(*m_vtbl)>;
#if STANDALONE_SYSTEM
    static vtbl_t vtbl{
        0,
        0,
        keyboard_get_id,
        keyboard_get_axis_count,
        keyboard_get_axis_id,
        keyboard_get_axis_state,
        keyboard_get_axis_old_state,
        keyboard_get_axis_delta,
        keyboard_poll,
        0,
        keyboard_finalize,
        keyboard_is_connected,
        keyboard_clear_state,
        keyboard_vibrate,
        keyboard_vibrate_scalar,
        keyboard_stop_vibration,
        keyboard_is_vibrator_present,
    };
#else
    Var<vtbl_t> tmp {0x0088EA80};
    static vtbl_t vtbl = tmp();
#endif

    this->m_vtbl = &vtbl;
    this->field_4 = INVALID_DEVICE_ID;

#if !STANDALONE_SYSTEM
    {
        FUNC_ADDRESS(address, &keyboard_device::_clear_state);
        this->m_vtbl->clear_state = CAST(this->m_vtbl->clear_state, address);
    }

    {
        FUNC_ADDRESS(address, &keyboard_device::_get_axis_state);
        this->m_vtbl->get_axis_state = CAST(this->m_vtbl->get_axis_state, address);
    }

    {
        FUNC_ADDRESS(address, &keyboard_device::_get_axis_old_state);
        this->m_vtbl->get_axis_old_state = CAST(this->m_vtbl->get_axis_old_state, address);
    }

    {
        FUNC_ADDRESS(address, &keyboard_device::_get_axis_delta);
        this->m_vtbl->get_axis_delta = CAST(this->m_vtbl->get_axis_delta, address);
    }

    {
        FUNC_ADDRESS(address, &keyboard_device::_poll);
        this->m_vtbl->poll = CAST(this->m_vtbl->poll, address);
    }

    {
        FUNC_ADDRESS(address, &keyboard_device::_is_connected);
        this->m_vtbl->is_connected = CAST(this->m_vtbl->is_connected, address);
    }
#endif
}

static keyboard_device g_device {};

keyboard_device * keyboard_device::instance = &g_device;

mString keyboard_device::get_name() const
{
    mString a1 {"USB Keyboard"};
    return a1;
}

void keyboard_device::clear()
{
    this->field_4 = INVALID_DEVICE_ID;
}

void keyboard_device::initialize(int a2)
{
    this->field_4 = 0x1E8480;
    this->field_8 = a2;
}

void keyboard_device::_clear_state()
{
    ;
}

float keyboard_device::_get_axis_state(int axis, [[maybe_unused]] int a3)
{
    TRACE("keyboard_device::get_axis_state");

    // sp_log("axis = %d, %d", axis, a3);

    if (Input::instance != nullptr) {
        static uint16_t key_codes[KB_NUM_AXES] {}; 

#define register_key_code(key)  key_codes[KB_##key] = DIK_##key

        register_key_code(I);
        register_key_code(K);
        register_key_code(T);
        register_key_code(J);
        register_key_code(L);
        register_key_code(O);
        register_key_code(U);
        register_key_code(RSHIFT);
        register_key_code(RALT);
        register_key_code(V);
        register_key_code(G);
        register_key_code(ADD);
        register_key_code(SUBTRACT);
        register_key_code(BACKSLASH);
        register_key_code(F9);

#undef register_key_code

        float result = (Input::instance->m_state_keys[key_codes[axis]] != 0);
        return result;
    }

    return 0.0f;
}

float keyboard_device::_get_axis_old_state(int, int)
{
    return 0.0f;
}

float keyboard_device::_get_axis_delta(int axis, int a2)
{
    return this->_get_axis_state(axis, a2);
}

void keyboard_device::_poll()
{
    TRACE("keyboard_device::poll");
    ;
}

bool keyboard_device::_is_connected() const
{
    return this->field_4 != INVALID_DEVICE_ID;
}

bool keyboard_device::_is_vibrator_present() const
{
    return false;
}
