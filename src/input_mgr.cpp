#include "input_mgr.h"

#include "common.h"
#include "func_wrapper.h"
#include "game.h"
#include "input.h"
#include "input_device.h"
#include "keyboard_device.h"
#include "memory.h"
#include "pc_input_mgr.h"
#include "pc_joypad_device.h"
#include "rumble_manager.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <cassert>

VALIDATE_SIZE(input_mgr, 0x5Cu);

VALIDATE_SIZE(input_mgr::control_map, 12);

#if !STANDALONE_SYSTEM

input_mgr *&input_mgr::instance = var<input_mgr *>(0x009685DC);

bool &pc_inserted_devices = var<bool>(0x00965EBD);

static uint32_t &PreviousConnected = var<uint32_t>(0x009874F4);

#else

input_mgr *&input_mgr::instance = []() -> auto & {
    static input_mgr *g_instance{};
    return g_instance;
}();

bool &pc_inserted_devices = []() -> auto & {
    static bool g_pc_inserted_devices{};
    return g_pc_inserted_devices;
}();

static uint32_t &PreviousConnected = []() -> auto & {
    static uint32_t g_PreviousConnected{};
    return g_PreviousConnected;
}();

#endif

bool IS_JOYSTICK_DEVICE(int id)
{
    return id >= 0xF4240 && id <= 0xF4247;
}

bool IS_KEYBOARD_DEVICE(int id)
{
    return id == 0x1E8480;
}

int DEVICE_ID_TO_KEYBOARD_INDEX(int id)
{
    return (id - 0x1E8480);
}

bool IS_MOUSE_DEVICE(device_id_t id)
{
    return id == 0x2DC6C0;
}

int DEVICE_ID_TO_MOUSE_INDEX(int id)
{
    return id - MOUSE1_DEVICE;
}


input_mgr::input_mgr()
{
    if constexpr (1) {
        this->m_vtbl = 0x0089165C;

        this->field_24 = false;
        this->field_25 = false;
        this->field_26 = false;
        this->field_58 = INVALID_DEVICE_ID;
        this->field_20 = 0;

        for (int i = 0; i < 8; ++i) {
            this->field_30[i] = nullptr;
        }

        this->keyboard_devices[0] = nullptr;
        this->mouse_devices[0] = nullptr;
        this->scan_devices();

        this->rumble_ptr = new rumble_manager{};
        this->field_24 = false;
        this->field_25 = false;
        this->field_26 = true;
        this->m_state_callback = nullptr;
        this->m_delta_callback = nullptr;
    } else {
        THISCALL(0x005E0EA0, this);
    }
}

input_mgr::~input_mgr()
{
    TRACE("input_mgr::~input_mgr");

    if constexpr (1) {
        this->m_vtbl = 0x0089165C;
        if (this->rumble_ptr != nullptr) {
            delete rumble_ptr;
        }

        this->rumble_ptr = nullptr;
    } else {
        THISCALL(0x005E0870, this);
    }
}

void *input_mgr::operator new(size_t size)
{
    return mem_alloc(size);
}

void input_mgr::operator delete(void *ptr, size_t size)
{
    return mem_dealloc(ptr, size);
}

void input_mgr::create_inst()
{
    TRACE("input_mgr::create_inst");

    if constexpr (1) {
        instance = new input_mgr{};
    } else {
        CDECL_CALL(0x005EB840);
    }
}

void input_mgr::register_control(const game_control &control)
{
    if constexpr (0) {
        _std::map<int, game_control> *map = &this->control_map;

        assert(map->find(control.name) == map->end());

        auto &v2 = map->operator[](control.name);

        v2 = control;
        //

    } else {
        THISCALL(0x005E0920, this, &control);
    }
}

BOOL __cdecl GetDeviceChanges([[maybe_unused]] void *a1, unsigned int *pdwInsertions, unsigned int *pdwRemovals)
{
    auto v3 = PreviousConnected;
    auto CurrentConnected = Input::instance->m_current_connected;
    *pdwInsertions = CurrentConnected & ~PreviousConnected;
    PreviousConnected = CurrentConnected;
    *pdwRemovals = v3 & ~CurrentConnected;
    return false;
}

void input_mgr::scan_devices()
{
    TRACE("input_mgr::scan_devices");

    if constexpr (1) {
        if (Input::instance == nullptr || Input::instance->m_din == nullptr) {
            keyboard_device::instance->clear();
        } else {
            keyboard_device::instance->initialize(0);
            this->insert_device(keyboard_device::instance);
        }
    }

    if constexpr (1) {
        if (!pc_inserted_devices) {
            pc_inserted_devices = true;
            for (auto i = 0u; i < 4u; ++i) {
                if (pc_input_mgr::instance->pad[i]) {
                    this->insert_device(pc_input_mgr::instance->pad[i]);
                }
            }
        }

        uint32_t dwInsertions;
        uint32_t dwRemovals;
        GetDeviceChanges(nullptr, &dwInsertions, &dwRemovals);
        for (auto j = 0u; j < 4u; ++j) {
            pc_input_mgr::instance->pad[j]->field_8C = pc_input_mgr::instance->pad[j]->field_88;
            if (((1 << j) & dwInsertions) != 0) {
                pc_input_mgr::instance->pad[j]->field_88 = 0;
                auto *v4 = pc_input_mgr::instance;
                v4->pad[j]->field_70 = InputOpen(0, j);
                InputGetCapabilities(pc_input_mgr::instance->pad[j]->field_70,
                                     &pc_input_mgr::instance->pad[j]->field_48);
                pc_input_mgr::instance->pad[j]->field_4 = j + 1000000;
                pc_input_mgr::instance->pad[j]->poll();
            }
        }

        if (this->field_58 == -1 || this->field_26) {
            for (auto v5 = 0u; v5 < 4u; ++v5) {
                if (pc_input_mgr::instance->pad[v5]->is_connected()) {
                    this->field_58 = static_cast<device_id_t>(v5 + 1000000);
                    if (g_game_ptr != nullptr) {
                        g_game_ptr->setup_inputs();
                    }
                }

                return;
            }
        }
    } else {
        THISCALL(0x00599090, this);
    }

    {
        using map_t = std::decay_t<decltype(this->control_map)>;

        void(__fastcall * find)(const map_t *, void *edx, map_t::iterator *, const int *a3) = CAST(find, 0x005E47A0);

        const int control = 14;
        map_t::iterator it;
        find(&this->control_map, nullptr, &it, &control);

        assert(it != this->control_map.end());

        assert(it->second.mapping.size() == 2u);

        assert(it->second.mapping.front().m_device_id == 0x1E8480);
    }
}

void input_mgr::frame_advance(Float a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x005DAB20, this, a2);
    }
}

float input_mgr::get_control_state(int control, device_id_t a3) const
{
    TRACE("input_mgr::get_control_state");
    sp_log("control = %d", control);

    if constexpr (0) {
#if 1
        auto it = this->control_map.find(control);
        assert(it != this->control_map.end());
#else

        auto it = [this, control]() {
            decltype(this->control_map)::iterator it;

            void(__fastcall * func)(const decltype(this->control_map) *, void *edx, decltype(it) *, const int *) =
                CAST(func, 0x005E47A0);
            func(&this->control_map, nullptr, &it, &control);
            assert(it != this->control_map.end());
            return it;
        }();
#endif

        auto &pair = *it;
        auto &control = pair.second;
        auto &list = control.mapping;
        float a1 = 0.0;
        bool v17 = false;
        auto v16 = list.size();

        for (auto &axis : list) {
            --v16;
            auto id = axis.m_device_id;
            if (a3 != INVALID_DEVICE_ID && a3 != id) {
                if (v17 || v16 != 0) {
                    continue;
                }

                id = a3;
            }

            v17 = true;
            auto *device = this->get_device_from_map(static_cast<device_id_t>(id));
            if (device != nullptr && device->is_connected()) {
                auto v12 = device->get_axis_state(axis.field_4, axis.field_8);
                if (control.type == CT_BOOLEAN) {
                    auto func = [](float a1) -> float {
                        if (a1 < -0.75) {
                            return -1.0;
                        }

                        if (a1 > 0.75) {
                            return 1.0;
                        }

                        return 0.0f;
                    };

                    v12 = func(v12);
                }

                a1 += v12;
            }
        }

        a1 = std::clamp(a1, -1.f, 1.f);
        return a1;
    } else {
        float(__fastcall * func)(const input_mgr *, void *edx, int, device_id_t) = CAST(func, 0x005D86D0);
        return func(this, nullptr, control, a3);
    }
}

void input_mgr::insert_device(input_device *a2)
{
    TRACE("input_mgr::insert_device");

    sp_log("0x%X", a2->get_id());

    if constexpr (1) {
        auto *v2 = a2;
        auto id = a2->get_id();

        if constexpr (0) {
            input_device **(__fastcall * insert)(void *, void *edx, const device_id_t *) = CAST(insert, 0x005E8400);
            auto found_device = insert(&this->device_map, nullptr, &id);
            *found_device = v2;
        } else {
            this->device_map[id] = v2;
        }

        if (IS_JOYSTICK_DEVICE(v2->get_id())) {
            *((DWORD *)&this[0xFFFF562B] + v2->get_id() - 0x11) = (DWORD)v2;
        } else if (IS_KEYBOARD_DEVICE(v2->get_id())) {
            this->keyboard_devices[DEVICE_ID_TO_KEYBOARD_INDEX(v2->get_id())] = v2;
        } else if (IS_MOUSE_DEVICE(v2->get_id())) {
            *((DWORD *)&this[0xFFFE027F] + v2->get_id() - 0x14) = (DWORD)v2;
        }
    } else {
        THISCALL(0x005DC140, this, a2);
    }
}

void input_mgr::set_control_state_monkey_callback(float (*a2)(int))
{
    this->m_state_callback = a2;
}

void input_mgr::set_control_delta_monkey_callback(float (*a2)(int))
{
    this->m_delta_callback = a2;
}

input_device *input_mgr::get_device_from_map_internal(device_id_t id) const
{
    if constexpr (0) {
        for (auto &p : this->device_map) {
            auto *d = p.second;
            if (d != nullptr) {
                if (d->get_id() == id) {
                    return d;
                }
            }
        }

        return nullptr;
    } else {
        input_device *(__fastcall * func)(const void *, void *edx, device_id_t id) = CAST(func, 0x005D59B0);
        return func(this, nullptr, id);
    }
}

void input_mgr::poll_devices()
{
    TRACE("input_mgr::poll_devices");

    sp_log("%d", this->device_map.size());

    if constexpr (0) {
        for (auto &dev : this->device_map) {
            dev.second->poll();
        }

    } else {
        THISCALL(0x005D5A60, this);
    }
}

float input_mgr::get_control_delta(int control, device_id_t a3) const
{
    TRACE("input_mgr::get_control_delta");
    sp_log("control = %d", control);

    if constexpr (1) {
        if (m_delta_callback != nullptr) {
            return m_delta_callback(control);
        }

        auto it = [this, control]() {
            _std::map<int, game_control>::iterator it;

            THISCALL(0x005E47A0, &this->control_map, &it, &control);

            assert(it != control_map.end());

            return it;
        }();

        auto &dalist = it->second.mapping;
        int size = dalist.size();
        bool v8 = false;
        float result = 0;

        for (auto it_list = dalist.begin(); it_list != dalist.end(); ++it_list) {
            --size;

            int id = it_list->m_device_id;

            if (a3 != INVALID_DEVICE_ID && a3 != id) {
                ++it_list;
                if (v8 || size != 0) {
                    continue;
                }

                id = a3;
            }

            v8 = true;

            if (input_device *v12 = this->get_device_from_map(static_cast<device_id_t>(id));
                v12 != nullptr && v12->is_connected()) {
                auto &axis = (*it_list);

                float v17;
                auto &pair = *it;
                auto &control = pair.second;
                if (control.type == CT_RATIONAL) {
                    v17 = v12->get_axis_delta(axis.field_4, axis.field_8);
                } else {
                    auto sub_C079D0 = [](float a1) -> float {
                        if (a1 < -0.75) {
                            return -1.0;
                        }

                        float v2 = (a1 <= 0.75 ? 0.0 : 1.0);
                        return v2;
                    };

                    auto axis_old_state = v12->get_axis_old_state(axis.field_4, axis.field_8);
                    auto v21 = sub_C079D0(axis_old_state);

                    auto axis_state = v12->get_axis_state(axis.field_4, axis.field_8);
                    auto v16 = sub_C079D0(axis_state);

                    auto v17 = v16 - v21;
                    if (v17 >= 0.0f) {
                        if (v17 > 0.0f) {
                            v17 = 0.0f;
                        }
                    } else {
                        v17 = -1.f;
                    }
                }

                result += v17;
            }
        }

        return result;
    } else {
        float(__fastcall * func)(const void *, void *edx, int control, device_id_t a3) = CAST(func, 0x005D87C0);
        return func(this, nullptr, control, a3);
    }
}

input_device *input_mgr::get_device_from_map(device_id_t id) const
{
    TRACE("input_mgr::get_device_from_map");

    if constexpr (1) {
        assert(id == INVALID_DEVICE_ID || IS_JOYSTICK_DEVICE(id) || IS_KEYBOARD_DEVICE(id) || IS_MOUSE_DEVICE(id));

        if (id <= 0x1E8480) {
            if (IS_KEYBOARD_DEVICE(id)) {
                return this->keyboard_devices[0];
            }

            if (IS_JOYSTICK_DEVICE(id)) {
                return (input_device *)*(bit_cast<uint32_t *>(&this[0xFFFF562B] + id - 0x11));
            }

            return this->get_device_from_map(id);
        }

        if (IS_MOUSE_DEVICE(id)) {
            return this->mouse_devices[0];
        }

        return this->get_device_from_map(id);

    } else {
        return (input_device *)THISCALL(0x0055E850, this, id);
    }
}

void input_mgr::clear_mapping()
{
    if constexpr (1) {
        for (auto &pair : this->control_map) {
            pair.second.type = {};
        }
    } else {
        THISCALL(0x005DC2A0, this);
    }
}

void input_mgr::map_control(int a2, device_id_t a3, int a4)
{
    TRACE("input_mgr::map_control(int, device_id_t, int)");

    if constexpr (1) {
        auto *v6 = this->get_device_from_map_internal(a3);
        if (v6 != nullptr) {
            device_axis v9;
            v9.m_device_id = a3;
            v9.field_8 = a4;
            v9.field_4 = v6->get_axis_id(a4);
            if (v9.field_4 != -1) {
                this->map_control(a2, v9);
            }
        }
    } else {
        THISCALL(0x005D8660, this, a2, a3, a4);
    }
}

void input_mgr::map_control(int a2, const device_axis &a3)
{
    TRACE("input_mgr::map_control");

    if constexpr (1) {
        auto it = this->control_map.find(a2);
        assert(it != this->control_map.end());

        auto &list = it._Ptr->_Myval.second.mapping;
        list.push_back(a3);
    } else {
        void(__fastcall * func)(void *, void *edx, int a2, const device_axis *a3) = CAST(func, 0x005D8610);
        func(this, nullptr, a2, &a3);
    }
}

void input_mgr_patch()
{
    {
        void (input_mgr::*func)(int, device_id_t, int) = &input_mgr::map_control;
        FUNC_ADDRESS(address, func);
        SET_JUMP(0x005D8660, address);
    }

    {
        FUNC_ADDRESS(address, &input_mgr::poll_devices);
        REDIRECT(0x00557C17, address);
        REDIRECT(0x005D7085, address);
        REDIRECT(0x00618D49, address);
    }

    {
        FUNC_ADDRESS(address, &input_mgr::insert_device);
        REDIRECT(0x005990C0, address);
    }

    {
        FUNC_ADDRESS(address, &input_mgr::scan_devices);
        REDIRECT(0x0055D696, address);
    }

    {
        FUNC_ADDRESS(address, &input_mgr::get_control_delta);
        //SET_JUMP(0x005D87C0, address);
    }

    {
        FUNC_ADDRESS(address, &input_mgr::get_control_state);
        REDIRECT(0x00528F42, address);
    }

    {
        FUNC_ADDRESS(address, &input_mgr::get_device_from_map);
        //REDIRECT(0x005D8734, address);
        //REDIRECT(0x005D884C, address);
    }
}
