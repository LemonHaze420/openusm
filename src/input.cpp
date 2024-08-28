#include "input.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "utility.h"
#include "variables.h"

#include <cstdio>

VALIDATE_SIZE(Input, 0x129E8u);

VALIDATE_OFFSET(Input, m_din, 0x27EC);

Input *& dword_965DDC = var<Input *>(0x00965DDC);

static Var<HRESULT(__stdcall *)(
    HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID *ppvOut, LPUNKNOWN punkOuter)>
    p_DirectInput8Create{0x00987944};

Input::Input()
{
    static Var<bool> din_intialized{0x00987950};

    this->field_0 = false;
    this->field_4 = 0;
    this->field_8 = false;
    this->m_initialized = false;
    this->m_sensitivity = 0.0099999998f;
    this->field_129D0 = 4;
    if (!din_intialized()) {
        char Buffer[260]{};

        if (GetSystemDirectoryA(Buffer, 260u)) {
            Buffer[259] = '\0';

            char Dest[268]{};
            std::sprintf(Dest, "%s\\dinput8.dll", Buffer);

            static Var<HMODULE> dinput8_dll{0x0098794C};

            auto v3 = LoadLibraryA(Dest);
            dinput8_dll() = v3;

            p_DirectInput8Create() = nullptr;
            if (v3) {
                p_DirectInput8Create() = CAST(p_DirectInput8Create(),
                                              GetProcAddress(v3, "DirectInput8Create"));
            }
        }

        din_intialized() = true;
    }

    this->m_di_keyboard = nullptr;
    this->m_di_mouse = nullptr;
    this->m_din = nullptr;

    std::memset(&this->m_mouse_state, 0, sizeof(this->m_mouse_state));

    std::memset(&this->m_old_mouse_state, 0, sizeof(this->m_old_mouse_state));

    std::memset(&this->field_1A38, 0, sizeof(this->field_1A38));

    std::memset(this->field_4F8, 0, sizeof(this->field_4F8));
    std::memset(this->field_F98, 0, sizeof(this->field_F98));
    std::memset(this->field_1A4C, 0, sizeof(this->field_1A4C));

    std::memset(this->field_14, 0, sizeof(this->field_14));

    std::memset(this->field_3C, 0, sizeof(this->field_3C));

    std::memset(this->field_64, 0, sizeof(this->field_64));

    std::memset(this->field_8C, 0, sizeof(this->field_8C));
    this->field_129D8[0] = nullptr;
    this->field_129D8[1] = nullptr;
    this->field_129D8[2] = nullptr;
    this->field_129D8[3] = nullptr;
    this->m_current_connected = 0;
}

Input::~Input() {
    THISCALL(0x0081FDD0, this);
}

void Input::set_mouse(const char *mouseLeft,
                      const char *mouseRight,
                      const char *mouseMiddle,
                      const char *mouseBtn,
                      const char *mouseAxis,
                      const char *mouseWheelUp,
                      const char *mouseWheelDown) {
    strcpy(this->m_mouseLeft, mouseLeft);
    strcpy(this->m_mouseRight, mouseRight);
    strcpy(this->m_mouseMiddle, mouseMiddle);
    strcpy(this->m_mouseBtn, mouseBtn);
    strcpy(this->m_mouseAxis, mouseAxis);
    strcpy(this->m_mouseWheelUp, mouseWheelUp);
    strcpy(this->m_mouseWheelDown, mouseWheelDown);
}

void Input::set_gamepad(const char *gamepadAxis, const char *gamepadPoV, const char *gamepadBtn) {
    strcpy(this->m_gamepadAxis, gamepadAxis);
    strcpy(this->m_gamepadPoV, gamepadPoV);
    strcpy(this->m_gamepadBtn, gamepadBtn);
}

const char *Input::get_string(InputType input_type, unsigned int Dest)
{
    if constexpr (1)
    {
        if (Dest == 0xFFFFFFFF || Dest == 0xFFFF) {
            return nullptr;
        }

        if (input_type == InputType::Key) {
            if (Dest >= 256) {
                return " ";
            }

            static char byte_987AC0[256]{};

            std::memcpy(byte_987AC0, this->m_keys[Dest], strlen(this->m_keys[Dest]));

            return byte_987AC0;
        }

        if (input_type == InputType::Mouse)
        {
            static char byte_987A70[80]{};
            std::memset(byte_987A70, 0, sizeof(byte_987A70));
            switch (Dest) {
            case 9u:
                return this->m_mouseLeft;
            case 10u:
                return this->m_mouseRight;
            case 11u:
                return this->m_mouseMiddle;
            case 12u:
                sprintf(byte_987A70, this->m_mouseBtn, "4");
                break;
            case 13u:
                sprintf(byte_987A70, this->m_mouseBtn, "5");
                break;
            case 14u:
                sprintf(byte_987A70, this->m_mouseBtn, "6");
                break;
            case 15u:
                sprintf(byte_987A70, this->m_mouseBtn, "7");
                break;
            case 16u:
                sprintf(byte_987A70, this->m_mouseBtn, "8");
                break;
            case 4u:
                sprintf(byte_987A70, this->m_mouseAxis, "X");
                break;
            case 3u:
                sprintf(byte_987A70, this->m_mouseAxis, "Y");
                break;
            default:
                if (Dest) {
                    switch (Dest) {
                    case 5u:
                        sprintf(byte_987A70, this->m_mouseAxis, "Y-");
                        break;
                    case 6u:
                        sprintf(byte_987A70, this->m_mouseAxis, "Y+");
                        break;
                    case 7u:
                        sprintf(byte_987A70, this->m_mouseAxis, "X-");
                        break;
                    case 8u:
                        sprintf(byte_987A70, this->m_mouseAxis, "X+");
                        break;
                    case 1u: {
                        return this->m_mouseWheelUp;
                    }
                    case 2u: {
                        return this->m_mouseWheelDown;
                    }
                    }
                } else {
                    sprintf(byte_987A70, this->m_mouseAxis, "Z");
                }

                break;
            }

            return byte_987A70;
        }

        if (input_type < InputType::Joy || input_type > InputType::Joy9) {
            return nullptr;
        }

        if (Dest >= 21 && Dest <= 49) {
            static char byte_987A20[80]{};
            std::memset(byte_987A20, 0, sizeof(byte_987A20));
            sprintf((char *) &Dest, "%d", Dest - 20);
            sprintf(byte_987A20, this->m_gamepadBtn, &Dest);
            return byte_987A20;
        }

        if (Dest == 0 || Dest > 16) {
            if (Dest >= 17u && Dest <= 20u) {
                static char byte_987980[80]{};
                std::memset(byte_987980, 0, sizeof(byte_987980));
                switch (Dest) {
                case 17u:
                    sprintf(byte_987980, this->m_gamepadPoV, "X+");
                    break;
                case 18u:
                    sprintf(byte_987980, this->m_gamepadPoV, "X-");
                    break;
                case 19u:
                    sprintf(byte_987980, this->m_gamepadPoV, "Y+");
                    break;
                default:
                    sprintf(byte_987980, this->m_gamepadPoV, "Y-");
                    break;
                }

                return byte_987980;
            }

            return nullptr;
        }

        static char byte_9879D0[80]{};
        std::memset(byte_9879D0, 0, sizeof(byte_9879D0));
        switch (Dest) {
        case 1u:
            sprintf(byte_9879D0, this->m_gamepadAxis, "LX+");
            return byte_9879D0;
        case 2u:
            sprintf(byte_9879D0, this->m_gamepadAxis, "LX-");
            return byte_9879D0;
        case 3u:
            sprintf(byte_9879D0, this->m_gamepadAxis, "LY+");
            return byte_9879D0;
        case 4u:
            sprintf(byte_9879D0, this->m_gamepadAxis, "LY-");
            return byte_9879D0;
        }

        auto v8 = Dest - 1;
        char tmp[8];
        if ((((BYTE) Dest - 1) & 1) != 0) {
            sprintf(tmp, "%d-", (v8 >> 1) + 1);
        } else {
            sprintf(tmp, "%d+", (v8 >> 1) + 1);
        }

        sprintf(byte_9879D0, this->m_gamepadAxis, tmp);
        return byte_9879D0;

    } else {
        auto *result = (char *) THISCALL(0x00820890, this, input_type, Dest);

        return result;
    }
}

void Input::sub_8203F0(int a2, InputSettings *a3) {
    if constexpr (1) {
        if (this->field_129D8[a2] != a3) {
            this->field_129D8[a2] = a3;
            if (a3->field_2) {
                RECT Rect;
                GetClientRect(this->m_hwnd, &Rect);

                POINT Point;
                Point.x = (Rect.right - Rect.left) / 2;
                Point.y = (Rect.bottom - Rect.top) / 2;
                ClientToScreen(this->m_hwnd, &Point);
                SetCursorPos(Point.x, Point.y);
            }

            std::memcpy(this->field_24EC, this->m_state_keys, sizeof(this->field_24EC));

            std::memcpy(&this->field_1A38, &this->m_mouse_state, sizeof(this->field_1A38));

            this->field_8 = true;
        }
    } else {
        THISCALL(0x008203F0, this, a2, a3);
    }
}

void Input::sub_81FC00(int a2, const char *a3) {
    THISCALL(0x0081FC00, this, a2, a3);
}

void Input::sub_81FB90(bool a2) {
    this->field_0 = a2;
}

void Input::set_key(int index, const char *a3) {
    strcpy(this->m_keys[index], a3);
}

bool Input::initialize(HWND a2)
{
    if constexpr (1) {
        if (this->m_initialized) {
            return true;
        }

        if (a2 == nullptr) {
            return false;
        }

        if (this->m_din == nullptr)
        {
            if (p_DirectInput8Create() == nullptr) {
                return false;
            }

            this->m_hwnd = a2;
            auto hModule = GetModuleHandleA(nullptr);

            p_DirectInput8Create()(hModule,
                                   DIRECTINPUT_VERSION,
                                   IID_IDirectInput8,
                                   (LPVOID *) &this->m_din,
                                   nullptr);
            this->sub_821490(false);
            BYTE KeyState[256]{};
            GetKeyboardState(KeyState);

            BYTE v11;
            int v6 = (char *) this - (char *) &v11 + 10404;
            for (uint32_t ScanCode = 0; ScanCode < 256; ++ScanCode)
            {
                const uint32_t VirtualKeyCode = MapVirtualKeyA(ScanCode, MAPVK_VSC_TO_VK);
                const uint32_t CharCode = MapVirtualKeyA(VirtualKeyCode, MAPVK_VK_TO_CHAR);
                BYTE v11 = CharCode;
                switch (CharCode) {
                case VK_LAUNCH_MAIL:
                    v11 = VK_RIGHT;
                    break;
                case VK_LWIN:
                    v11 = VK_BROWSER_FAVORITES;
                    break;
                case VK_APPS:
                    v11 = VK_BROWSER_HOME;
                    break;
                case VK_SLEEP:
                    v11 = VK_BROWSER_SEARCH;
                    break;
                case VK_OEM_1:
                    v11 = VK_MEDIA_NEXT_TRACK;
                    break;
                }

                auto *v9 = &v11;

                BYTE v10;
                do {
                    v10 = *v9;
                    v9[v6] = *v9;
                    ++v9;
                } while (v10);

                v6 += 256;
            }

            strcpy(this->m_keys[DIK_ESCAPE], "ESC");
            strcpy(this->m_keys[DIK_BACK], "BACK");
            strcpy(this->m_keys[DIK_TAB], "TAB");
            strcpy(this->m_keys[DIK_RETURN], "ENTER");
            strcpy(this->m_keys[DIK_RCONTROL], "RCTRL");
            strcpy(this->m_keys[DIK_LCONTROL], "LCTRL");
            strcpy(this->m_keys[DIK_LSHIFT], "LSHIFT");
            strcpy(this->m_keys[DIK_RSHIFT], "RSHIFT");
            strcpy(this->m_keys[DIK_SPACE], "SPACE");
            strcpy(this->m_keys[DIK_CAPITAL], "CAPSL");
            strcpy(this->m_keys[DIK_SCROLL], "SCROLL");
            strcpy(this->m_keys[DIK_LMENU], "LALT");
            strcpy(this->m_keys[DIK_RMENU], "RALT");
            strcpy(this->m_keys[DIK_PAUSE], "PAUSE");
            strcpy(this->m_keys[DIK_HOME], "HOME");
            strcpy(this->m_keys[DIK_PRIOR], "PGUP");
            strcpy(this->m_keys[DIK_UP], "UP");
            strcpy(this->m_keys[DIK_LEFT], "LEFT");
            strcpy(this->m_keys[DIK_DOWN], "DOWN");
            strcpy(this->m_keys[DIK_RIGHT], "RIGHT");
            strcpy(this->m_keys[DIK_DELETE], "DEL");
            strcpy(this->m_keys[DIK_INSERT], "INS");
            strcpy(this->m_keys[DIK_END], "END");
            strcpy(this->m_keys[DIK_NEXT], "PGDWN");
            strcpy(this->m_keys[DIK_NUMPADCOMMA], "KP,");
            strcpy(this->m_keys[DIK_DECIMAL], "KP.");
            strcpy(this->m_keys[DIK_NUMPADENTER], "KPEnter");
            strcpy(this->m_keys[DIK_DIVIDE], "/");
            strcpy(this->m_keys[DIK_SUBTRACT], "-");
            strcpy(this->m_keys[DIK_NUMLOCK], "NumL");
            strcpy(this->m_keys[DIK_ADD], "+");
            strcpy(this->m_keys[DIK_MULTIPLY], "*");
            strcpy(this->m_keys[DIK_NUMPAD0], "KP0");
            strcpy(this->m_keys[DIK_NUMPAD1], "KP1");
            strcpy(this->m_keys[DIK_NUMPAD2], "KP2");
            strcpy(this->m_keys[DIK_NUMPAD3], "KP3");
            strcpy(this->m_keys[DIK_NUMPAD4], "KP4");
            strcpy(this->m_keys[DIK_NUMPAD5], "KP5");
            strcpy(this->m_keys[DIK_NUMPAD6], "KP6");
            strcpy(this->m_keys[DIK_NUMPAD7], "KP7");
            strcpy(this->m_keys[DIK_NUMPAD8], "KP8");
            strcpy(this->m_keys[DIK_NUMPAD9], "KP9");
            strcpy(this->m_keys[DIK_F1], "F1");
            strcpy(this->m_keys[DIK_F2], "F2");
            strcpy(this->m_keys[DIK_F3], "F3");
            strcpy(this->m_keys[DIK_F4], "F4");
            strcpy(this->m_keys[DIK_F5], "F5");
            strcpy(this->m_keys[DIK_F6], "F6");
            strcpy(this->m_keys[DIK_F7], "F7");
            strcpy(this->m_keys[DIK_F8], "F8");
            strcpy(this->m_keys[DIK_F9], "F9");
            strcpy(this->m_keys[DIK_F10], "F10");
            strcpy(this->m_keys[DIK_F11], "F11");
            strcpy(this->m_keys[DIK_F12], "F12");
            strcpy(this->m_keys[DIK_F13], "F13");
            strcpy(this->m_keys[DIK_F14], "F14");
            strcpy(this->m_keys[DIK_F15], "F15");
            this->m_initialized = true;
        }

        return true;
    }
    else
    {
        bool result = (bool) THISCALL(0x00821800, this, a2);

        return result;
    }
}

int Input::sub_8200C0(const DIJOYSTATE2 *a1, int a2)
{
    int result;

    bool v12;

    switch (a2) {
    case 1:
        return (a1->lX <= 0 ? 0 : a1->lX);
    case 2: {
        if (a1->lX >= 0) {
            return 0;
        }

        return -a1->lX;
    }
    case 3: {
        return (a1->lY <= 0 ? 0 : a1->lY);
    }
    case 4: {
        auto v4 = a1->lY;
        if (v4 >= 0) {
            return 0;
        }

        return -v4;
    }
    case 5: {
        return (a1->lZ <= 0 ? 0 : a1->lZ);
    }
    case 6: {
        auto v5 = a1->lZ;
        if (v5 >= 0) {
            return 0;
        }

        return -v5;
    }
    case 7: {
        return (a1->lRx <= 0 ? 0 : a1->lRx);
    }
    case 8: {
        auto v6 = a1->lRx;
        if (v6 >= 0) {
            return 0;
        }

        return -v6;
    }
    case 9: {
        return (a1->lRy <= 0 ? 0 : a1->lRy);
    }
    case 10: {
        auto v7 = a1->lRy;
        if (v7 >= 0) {
            return 0;
        }

        return -v7;
    }
    case 11:
        return (a1->lRz <= 0 ? 0 : a1->lRz);
    case 12: {
        auto v8 = a1->lRz;
        if (v8 >= 0) {
            return 0;
        }

        return -v8;
    }
    case 13: {
        return (a1->rglSlider[0] <= 0 ? 0 : a1->rglSlider[0]);
    }
    case 14: {
        auto v9 = a1->rglSlider[0];
        if (v9 >= 0) {
            return 0;
        }

        return -v9;
    }
    case 15: {
        return (a1->rglSlider[1] <= 0 ? 0 : a1->rglSlider[1]);
    }
    case 16: {
        auto v10 = a1->rglSlider[1];
        if (v10 >= 0) {
            return 0;
        }

        return -v10;
    }
    case 17: {
        if (LOWORD(a1->rgdwPOV[0]) == 0xFFFF) {
            return 0;
        }

        auto v11 = a1->rgdwPOV[0];
        if (v11 < 0x8CA) {
            return 0;
        }

        v12 = v11 <= 0x3D86;

        break;
    }
    case 18: {
        if (LOWORD(a1->rgdwPOV[0]) == 0xFFFF) {
            return 0;
        }

        auto v13 = a1->rgdwPOV[0];
        if (v13 < 0x4F1A) {
            return 0;
        }

        v12 = v13 <= 0x83D6;

        break;
    }
    case 19: {
        if (LOWORD(a1->rgdwPOV[0]) == 0xFFFF) {
            return 0;
        }

        auto v14 = a1->rgdwPOV[0];
        if (v14 < 0x2BF2) {
            return 0;
        }

        v12 = v14 <= 0x60AE;
        break;
    }
    case 20: {
        if (LOWORD(a1->rgdwPOV[0]) == 0xFFFF) {
            return 0;
        }

        auto v15 = a1->rgdwPOV[0];
        if (v15 >= 0x7242) {
            return 1000;
        }

        v12 = v15 <= 0x1A5E;

        break;
    }
    default: {
        result = 0;
    }
    }

    if (v12) {
        result = 1000;
    } else {
        result = 0;
    }

    return result;
}

int Input::sub_820080() {
    auto result = 0;
    for (auto *i = this->field_14; (*i) != 0; ++i) {
        if (++result >= 10) {
            return 10;
        }
    }

    return result;
}

float Input::get_joy_state(int a2, int a3)
{
    float result;

    if (!this->field_4EC[a2]) {
        return 0.0f;
    }

    if (a3 < 21 || a3 > 49) {
        if (a3 >= 1 && a3 <= 20) {
            return this->sub_8200C0(&this->field_4F8[a2], a3) * 0.001f;
        }

        return 0.0f;
    }

    if (*(bit_cast<char *>(&this->field_4F8[a2].rglSlider) + a3 + 3)) {
        result = 1.f;
    } else {
        result = 0.0f;
    }
    return result;
}

float Input::get_mouse_state(InputMouse a2, int mode)
{
    if constexpr (1)
    {
        auto *input_settings = this->field_129D8[mode];
        auto rotation_axis_y = this->m_mouse_state.lY * this->m_sensitivity;
        auto rotation_axis_x = this->m_mouse_state.lX * this->m_sensitivity;
        if (input_settings != nullptr && input_settings->field_3)
        {
            rotation_axis_x = (rotation_axis_x * input_settings->field_14 +
                               input_settings->field_C) *
                input_settings->field_4;
            rotation_axis_y = (rotation_axis_y * input_settings->field_14 +
                               input_settings->field_10) *
                input_settings->field_4;

            if (rotation_axis_x >= input_settings->field_8) {
                rotation_axis_x = input_settings->field_8;
            }

            auto v4 = -input_settings->field_8;
            if (rotation_axis_x <= v4) {
                rotation_axis_x = v4;
            }

            if (rotation_axis_y >= input_settings->field_8) {
                rotation_axis_y = input_settings->field_8;
            }

            if (rotation_axis_y <= v4) {
                rotation_axis_y = v4;
            }

            input_settings->field_C = rotation_axis_x;
            input_settings->field_10 = rotation_axis_y;
        }

        switch (a2) {
        case InputMouse::LeftButton: {
            return (this->m_mouse_state.rgbButtons[0] != 0);
        }
        case InputMouse::RightButton: {
            return (this->m_mouse_state.rgbButtons[1] != 0);
        }
        case InputMouse::MiddleButton: {
            return (this->m_mouse_state.rgbButtons[2] != 0);
        }
        case InputMouse::Unknown: {
            auto v6 = this->m_mouse_state.lZ;
            if (v6 <= 1) {
                if (v6 < -1) {
                    return -1.f;
                }

                return 0.0f;
            }

            return 1.0f;
        }
        case InputMouse::Unknown1: {
            if (this->m_mouse_state.lZ > 0) {
                return 1.0f;
            }

            break;
        }
        case InputMouse::Unknown2: {
            if (this->m_mouse_state.lZ < 0) {
                return 1.0f;
            }

            break;
        }
        case InputMouse::Unknown3: {
            return rotation_axis_y;
        }
        case InputMouse::Unknown4: {
            return rotation_axis_x;
        }
        case InputMouse::LookUp: {
            if (rotation_axis_y < 0.0f) {
                return (-rotation_axis_y);
            }

            break;
        }
        case InputMouse::LookDown: {
            if (rotation_axis_y > 0.0f) {
                return rotation_axis_y;
            }

            break;
        }
        case InputMouse::LookLeft: {
            if (rotation_axis_x < 0.0f) {
                return rotation_axis_x * (-1.f);
            }

            break;
        }
        case InputMouse::LookRight: {
            if (rotation_axis_x > 0.0f) {
                return rotation_axis_x;
            }

            break;
        }
        default: {
            break;
        }
        }

        return 0.0f;
    }
    else
    {
        float (__fastcall *func)(void *, void *, int, int) = CAST(func, 0x0081FE60);

        return func(this, nullptr, static_cast<int>(a2), mode);
    }
}

void Input::sub_8204C0(InputSettings::internal_struct *a2, int mode)
{
    if constexpr (1)
    {
        float state = 0.0f;

        for (auto i = 0u; i < a2->m_size; ++i)
        {
            for (auto v6 = 0u; v6 < 6u; ++v6)
            {
                auto action_index = i;

                auto &v5 = a2->field_4[action_index][v6];

                auto v7 = v5.m_input_type;

                auto value = v5.m_value;
                if (v7 != 0)
                {
                    switch (v7) {
                    case InputType::Key: {
                        state = (this->m_state_keys[value] != 0);

                        break;
                    }
                    case InputType::Mouse: {
                        state = this->get_mouse_state(static_cast<InputMouse>(value), mode);
                        break;
                    }
                    case InputType::Joy:
                    case InputType::Joy1:
                    case InputType::Joy2:
                    case InputType::Joy3:
                    case InputType::Joy4:
                    case InputType::Joy5:
                    case InputType::Joy6:
                    case InputType::Joy7:
                    case InputType::Joy8:
                    case InputType::Joy9: {
                        state = this->get_joy_state(v7 - 3, value);
                        break;
                    }
                    default:
                        break;
                    }

                    v5.field_8 = state;
                }
            }
        }
    }
    else
    {
        THISCALL(0x008204C0, this, a2, mode);
    }
}

bool key_pressed = false;

void Input::poll()
{
    if constexpr (0)
    {
        auto *v2 = this->m_di_keyboard;
        auto *diKeys = this->m_state_keys;
        std::memcpy(this->m_old_state_keys, this->m_state_keys, sizeof(this->m_old_state_keys));
        auto v4 = v2->lpVtbl->GetDeviceState(v2, 256, this->m_state_keys);

        if (v4 == DIERR_INPUTLOST || v4 == DIERR_NOTACQUIRED) {
            auto v5 = this->m_di_keyboard->lpVtbl->Acquire(this->m_di_keyboard);
            if (v5 < 0) {
                memset(diKeys, 0, 256u);
            } else {
                v2->lpVtbl->GetDeviceState(this->m_di_keyboard, 256, diKeys);
            }
        }

        auto *v6 = this->m_di_mouse;
        if (v6 != nullptr) {
            HRESULT v7;
            if (v6->lpVtbl->Poll(this->m_di_mouse) < 0 &&
                ((v7 = this->m_di_mouse->lpVtbl->Acquire(this->m_di_mouse), v7 < 0) ||
                 v6->lpVtbl->Poll(this->m_di_mouse) < 0)) {
                this->m_old_mouse_state = this->m_mouse_state;

                std::memset(&this->m_mouse_state, 0, sizeof(this->m_mouse_state));

            } else {
                this->m_old_mouse_state = this->m_mouse_state;

                this->m_di_mouse->lpVtbl->GetDeviceState(this->m_di_mouse, 20, &this->m_mouse_state);
            }

            if (this->field_0) {
                Var<int> dword_987BC4{0x00987BC4};

                auto v11 = this->m_mouse_state.lX == 0;

                if (v11 && dword_987BC4()) {
                    this->m_mouse_state.lX = dword_987BC4() / 2;
                }

                v11 = this->m_mouse_state.lY == 0;
                dword_987BC4() = this->m_mouse_state.lX;

                Var<int> dword_987BC0{0x00987BC0};

                if (v11 && dword_987BC0()) {
                    this->m_mouse_state.lY = dword_987BC0() / 2;
                }

                dword_987BC0() = this->m_mouse_state.lY;
            }
        }

        this->m_current_connected = 0;

        auto *v12 = &this->field_4F8[0];
        auto *v44 = this->field_14;

        for (auto v47 = 0u; v47 < 10u; ++v47)
        {
            auto v13 = *v44;
            if (*v44)
            {
                std::memcpy((int *) v12 + 678, v12, 0x110u);
                if ( ((*(int(__stdcall **)(int))(*(uint32_t *) v13 + 100))(v13) < 0 &&
                        (*(int(__stdcall **)(int))(*(uint32_t *) *v44 + 28))(*v44) < 0) ||
                         (*(int(__stdcall **)(int))(*(uint32_t *) *v44 + 100))(*v44) < 0 ||
                    (*(int(__stdcall **)(int, int, void *))(*(uint32_t *) *v44 +
                                                            36))(*v44, 272, v12) < 0)
                {
                    std::memset(v12, 0, sizeof(DIJOYSTATE2));
                    v12->rgdwPOV[0] = 0xFFFF;
                    v12->rgdwPOV[1] = 0xFFFF;
                    v12->rgdwPOV[2] = 0xFFFF;
                    v12->rgdwPOV[3] = 0xFFFF;
                }
                else
                {
                    auto v19 = this->field_4;
                    if (v19 > 0)
                    {
                        if (v12->lX > -v19 && v12->lX < v19) {
                            v12->lX = 0;
                        }

                        if (v12->lY > -this->field_4 && v12->lY < this->field_4) {
                            v12->lY = 0;
                        }

                        auto v20 = v12->lZ;
                        if (v20 > -this->field_4 && v20 < this->field_4) {
                            v12->lZ = 0;
                        }

                        auto v21 = v12->lRx;
                        if (v21 > -this->field_4 && v21 < this->field_4) {
                            v12->lRx = 0;
                        }

                        auto v22 = v12->lRy;
                        if (v22 > -this->field_4 && v22 < this->field_4) {
                            v12->lRy = 0;
                        }

                        auto v23 = v12->lRz;
                        if (v23 > -this->field_4 && v23 < this->field_4) {
                            v12->lRz = 0;
                        }
                    }

                    this->m_current_connected |= 1 << v47;
                }
            }

            ++v12;

            ++v44;
        }

        if (this->field_8)
        {
            auto *v16 = this->field_24EC;
            bool v17 = false;
            int v18 = 256;
            do {
                if (*v16)
                {
                    if (v16[256] == *v16) {
                        v16[256] = 0;
                        v17 = true;
                    } else {
                        *v16 = 0;
                    }
                }

                ++v16;
                --v18;
            } while (v18);

            auto v24 = this->field_1A38.lX;
            if (this->m_mouse_state.lX == v24 && v24) {
                this->m_mouse_state.lX = 0;
                v17 = true;
            } else {
                this->field_1A38.lX = 0;
            }

            auto v25 = this->field_1A38.lY;
            if (this->m_mouse_state.lY == v25 && v25) {
                this->m_mouse_state.lY = 0;
                v17 = 1;
            } else {
                this->field_1A38.lY = 0;
            }

            auto v26 = this->field_1A38.lZ;
            if (this->m_mouse_state.lZ == v26 && v26) {
                this->m_mouse_state.lZ = 0;
                v17 = 1;
            } else {
                this->field_1A38.lZ = 0;
            }

            auto *v27 = this->field_1A38.rgbButtons;
            auto v28 = 8;
            do {
                if (*(v27 - 5492) == *v27 && *v27) {
                    *(v27 - 5492) = 0;
                    v17 = true;
                } else {
                    *v27 = 0;
                }

                ++v27;
                --v28;
            } while (v28);

            auto *v29 = this->field_14;
            auto *v30 = this->field_1A4C;
            auto *v48 = this->field_14;
            auto v45 = 10;

            bool v11;
            do {
                if (*v29)
                {
                    if (*(v30 - 1365) == *v30 && *v30) {
                        *(v30 - 1365) = 0;
                        v17 = 1;
                    } else {
                        *v30 = 0;
                    }

                    auto v31 = v30[1];
                    if (*(v30 - 1364) == v31 && v31) {
                        *(v30 - 1364) = 0;
                        v17 = 1;
                    } else {
                        v30[1] = 0;
                    }

                    auto v32 = v30[2];
                    if (*(v30 - 1363) == v32 && v32) {
                        *(v30 - 1363) = 0;
                        v17 = 1;
                    } else {
                        v30[2] = 0;
                    }

                    auto v33 = v30[3];
                    if (*(v30 - 1362) == v33 && v33) {
                        *(v30 - 1362) = 0;
                        v17 = 1;
                    } else {
                        v30[3] = 0;
                    }

                    auto v34 = v30[4];
                    if (*(v30 - 1361) == v34 && v34) {
                        *(v30 - 1361) = 0;
                        v17 = 1;
                    } else {
                        v30[4] = 0;
                    }

                    auto v35 = v30[5];
                    if (*(v30 - 1360) == v35 && v35) {
                        *(v30 - 1360) = 0;
                        v17 = 1;
                    } else {
                        v30[5] = 0;
                    }

                    auto *v36 = v30 + 6;
                    auto v37 = 2;
                    do {
                        if (*(v36 - 1365) == *v36 && *v36) {
                            *(v36 - 1365) = 0;
                            v17 = 1;
                        } else {
                            *v36 = 0;
                        }
                        ++v36;
                        --v37;
                    } while (v37);

                    auto *v38 = v30 + 8;
                    auto v39 = 4;
                    do {
                        if (*(v38 - 1365) != *v38 || *v38 == -1) {
                            *v38 = 0xFFFF;
                        } else {
                            *(v38 - 1365) = 0xFFFF;
                            v17 = true;
                        }

                        ++v38;
                        --v39;
                    } while (v39);

                    for (auto i = 0; i < 30; ++i) {
                        auto v41 = *((BYTE *) v30 + i + 48);
                        if (*((BYTE *) v30 + i - 5412) == v41 && v41) {
                            *((BYTE *) v30 + i - 5412) = 0;
                            v17 = 1;
                        } else {
                            *((BYTE *) v30 + i + 48) = 0;
                        }
                    }
                }

                v29 = v48 + 1;
                v30 += 68;
                v11 = v45 == 1;
                ++v48;
                --v45;
            } while (!v11);

            if (!v17) {
                this->field_8 = false;
            }
        }

        auto **v43 = &this->field_129D8[0];
        for (auto i = 0u; i < this->field_129D0; ++i)
        {
            if (v43[i] != nullptr) {
                this->sub_8204C0(&v43[i]->field_18, i);
            }
        }
    }
    else
    {
        THISCALL(0x00820C70, this);
    }
}

BOOL __stdcall sub_821470(const DIDEVICEINSTANCEA *a2, void *arg4) {
    return (BOOL) STDCALL(0x00821470, a2, arg4);
}

static Var<int[4]> dword_8C0AAC{0x008C0AAC};

void Input::sub_821490(bool a2) {
    if constexpr (0) {
        std::memset(this->field_4EC, 0, sizeof(this->field_4EC));
        this->field_4F0 = 0;
        this->field_4F4 = 0;
        auto v3 = !a2;
        this->field_9 = a2;
        if (!v3) {
            this->m_din->lpVtbl->EnumDevices(this->m_din, 4, sub_821470, this, 1);
        }

        IDirectInputDevice8A *tmp = nullptr;

        if (this->m_di_keyboard == nullptr &&
            this->m_din->lpVtbl->CreateDevice(this->m_din, GUID_SysKeyboard, &tmp, nullptr) >= 0) {
            auto *v4 = tmp;
            this->m_di_keyboard = tmp;
            v4->lpVtbl->SetDataFormat(v4, &c_dfDIKeyboard);
            v4->lpVtbl->SetCooperativeLevel(v4, this->m_hwnd, 22);
            v4->lpVtbl->Acquire(v4);
        }
        if (this->m_di_mouse == nullptr &&
            this->m_din->lpVtbl->CreateDevice(this->m_din, GUID_SysMouse, &tmp, nullptr) >= 0) {
            auto *v5 = tmp;
            this->m_di_mouse = tmp;
            v5->lpVtbl->SetDataFormat(v5, &c_dfDIMouse2);
            v5->lpVtbl->SetCooperativeLevel(v5, this->m_hwnd, 6);
            v5->lpVtbl->Acquire(v5);
        }

        int v6 = 0;
        int v24 = 0;
        a2 = (BOOL) & this->field_8C[1];
        auto *v7 = this->field_3C;
        auto *v8 = this->field_F98;
        do {
            if (!this->field_4EC[v6] && *(v7 - 10)) {
                memset(field_4F8, 0, sizeof(this->field_F98));
                memset(v8, 0, sizeof(this->field_F98));
                (*(void(__stdcall **)(uint32_t))(*(uint32_t *) *(v7 - 10) + 8))(*(v7 - 10));
                auto v9 = *v7;
                v3 = *v7 == 0;
                *(v7 - 10) = 0;
                if (!v3) {
                    (*(void(__stdcall **)(int))(*(uint32_t *) v9 + 8))(v9);
                }

                auto v10 = (uint32_t *) a2;
                *(uint32_t *) (a2 - 4) = 0;
                *v10 = 0;
                v6 = v24;
                *v7 = 0;
                v7[10] = 0;
            }
            ++v6;
            ++v7;
            v8 += 68;
            v24 = v6;
            a2 += 8;
        } while (v6 < 10);

        int i;
        for (i = 0; i < 10; ++i) {
            if (!this->field_4EC[i]) {
                break;
            }
        }

        if (i < 10) {
            auto *v25 = &this->field_8C[2 * i + 1];
            auto *v23 = (char *) &this->field_8C[25 * i + 20];
            auto *v12 = &this->field_F98[i].lX;
            auto v29 = -1259 - (uint32_t) this;
            auto *v13 = this->field_27F0[i];
            auto *v14 = &this->field_4EC[i];
            auto *v26 = v14;
            auto *v15 = &this->field_3C[i];

            auto v27 = v12;
            auto v28 = 10 - i;
            do {
                if (!*v14) {
                    auto *v16 = &v14[v29];
                    if ((int) &v14[v29] < 10) {
                        while (!v16[(uint32_t) this + 1260]) {
                            if ((int) ++v16 >= 10) {
                                goto LABEL_27;
                            }
                        }

                        *v14 = 1;
                        v16[(uint32_t) this + 1260] = 0;
                        memset(v12 - 680, 0, sizeof(DIJOYSTATE2));
                        memset(v12, 0, sizeof(DIJOYSTATE2));
                        auto *v17 = (char *) &this->field_8C[25 * (uint32_t) v16 + 20];
                        strcpy(v23, v17);
                        memset(v17, 0, 0x64u);
                        auto *v18 = v13;
                        auto *v19 = (int *) (&this->field_0 + 16 * (uint32_t) (v16 + 639));
                        v18[0] = v19[0];
                        v18[1] = v19[1];
                        v18[2] = v19[2];
                        v18[3] = v19[3];

                        v19[0] = dword_8C0AAC()[0];
                        v19[1] = dword_8C0AAC()[1];
                        v19[2] = dword_8C0AAC()[2];
                        v12 = v27;
                        v19[3] = dword_8C0AAC()[3];
                        *(v15 - 10) = this->field_14[(uint32_t) v16];
                        auto v20 = this->field_3C[(uint32_t) v16];
                        this->field_14[(uint32_t) v16] = 0;
                        *v15 = v20;
                        auto v21 = this->field_64[(uint32_t) v16];
                        this->field_3C[(uint32_t) v16] = 0;
                        v15[10] = v21;
                        auto v22 = this->field_8C[2 * (uint32_t) v16];
                        this->field_64[(uint32_t) v16] = 0;
                        *(v25 - 1) = v22;
                        *v25 = this->field_8C[2 * (uint32_t) v16 + 1];

                        v14 = v26;
                        this->field_8C[2 * (uint32_t) v16] = 0;
                        this->field_8C[2 * (uint32_t) v16 + 1] = 0;
                    }
                }

            LABEL_27:
                v13 += 4;
                v23 += 100;
                ++v15;
                ++v14;
                v12 += 68;
                v3 = v28 == 1;

                v26 = v14;
                v27 = v12;
                v25 += 2;
                --v28;
            } while (!v3);
        }
        this->field_9 = 0;

    } else {
        THISCALL(0x00821490, this, a2);
    }
}

void Input::sub_820C60() {
    THISCALL(0x00820C60, this);
}

DIJOYSTATE2 *Input::sub_820570(int a2) {
    return &this->field_4F8[a2];
}

bool Input::sub_820590(int a2) {
    bool result = false;

    if (this->field_14[a2]) {
        result = (this->field_64[a2] > 0);
    }

    return result;
}

char * Input::sub_81FD40(int a2)
{
    static char byte_987954[44] {};
    uint8_t *v2 = bit_cast<uint8_t *>(this) + 16 * a2;
    sprintf(
            byte_987954,
            "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
            this->field_27F0[a2][0],
            *((uint16_t *)v2 + 5114),
            *((uint16_t *)v2 + 5115),
            v2[10232],
            v2[10233],
            v2[10234],
            v2[10235],
            v2[10236],
            v2[10237],
            v2[10238],
            v2[10239]);
    return byte_987954;
}

void Input::create_inst() {
    instance = new Input{};
}

void Input_patch() {
    FUNC_ADDRESS(address, &Input::get_string);
    REDIRECT(0x00587472, address);
    REDIRECT(0x0059565F, address);
    REDIRECT(0x00595676, address);
    REDIRECT(0x0059568D, address);
    REDIRECT(0x005A9CF0, address);
    REDIRECT(0x005A9D03, address);
    REDIRECT(0x005A9D21, address);

    {
        FUNC_ADDRESS(address, &Input::poll);
        REDIRECT(0x0081D24A, address);
        REDIRECT(0x005A505E, address);
        REDIRECT(0x005A9C5E, address);
    }

    {
        FUNC_ADDRESS(address, &Input::sub_8204C0);
        REDIRECT(0x008211A3, address);
    }
}
