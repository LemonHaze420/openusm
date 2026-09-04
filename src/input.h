#pragma once

#include "inputsettings.h"
#include "variable.h"

#include <windows.h>

#include <dinput.h>

struct IDirectInput8A;
struct InputSettings;
struct IDirectInputDevice8A;

struct Input {
    bool field_0;
    char empty0[3];
    int field_4;
    bool field_8;
    bool field_9;
    char empty1[2];
    IDirectInputDevice8A *m_di_keyboard;
    IDirectInputDevice8A *m_di_mouse;
    int field_14[10];
    int field_3C[10];
    int field_64[10];
    int field_8C[20];
    int empty2[250];
    DIMOUSESTATE2 m_mouse_state;
    DIMOUSESTATE2 m_old_mouse_state;
    char field_4EC[4];
    int field_4F0;
    int field_4F4;

    DIJOYSTATE2 field_4F8[10];
    DIJOYSTATE2 field_F98[10];
    DIMOUSESTATE2 field_1A38;
    int field_1A4C[680];
    char field_24EC[256];
    char m_state_keys[256];
    char m_old_state_keys[256];
    IDirectInput8A *m_din;
    int field_27F0[10][4];
    bool m_initialized;
    char empty6[3];
    HWND m_hwnd;
    float m_sensitivity;
    int empty26[2];
    char m_keys[222][256];
    int field_106A4;

    int empty27[2175];
    char m_mouseLeft[30];
    char m_mouseMiddle[30];
    char m_mouseRight[30];
    char m_mouseBtn[30];
    char m_mouseAxis[30];
    char m_mouseWheelUp[30];
    char m_mouseWheelDown[30];

    char m_gamepadAxis[30];
    char m_gamepadBtn[30];
    char m_gamepadPoV[30];
    uint32_t field_129D0;
    uint32_t m_current_connected;
    InputSettings *field_129D8[4];

    //0x0081F9A0
    Input();

    ~Input();

    //0x00820840
    void set_gamepad(const char *gamepadAxis, const char *gamepadPoV, const char *gamepadBtn);

    //0x00820790
    void set_mouse(const char *mouseLeft, const char *mouseRight, const char *mouseMiddle, const char *mouseBtn,
                   const char *mouseAxis, const char *mouseWheelUp, const char *mouseWheelDown);
    //0x00820760
    void set_key(int index, const char *a3);

    int sub_8200C0(const DIJOYSTATE2 *a1, int a2);

    int sub_820080();

    float get_joy_state(int a2, int a3);

    float get_mouse_state(InputMouse a2, int mode);

    void sub_8204C0(InputSettings::internal_struct *a2, int mode);

    void poll();

    //0x00820890
    const char *get_string(InputType input_type, unsigned int Dest);

    void sub_81FC00(int a2, const char *a3);

    void sub_81FB90(bool a2);

    void sub_8203F0(int a2, InputSettings *a3);

    bool initialize(HWND a2);

    void sub_821490(bool a2);

    void sub_820C60();

    DIJOYSTATE2 *sub_820570(int a2);

    bool sub_820590(int a2);

    char *sub_81FD40(int a2);

    //0x005B0D00
    static void create_inst();

    static inline constexpr std::pair<int, const char *> keyConstants[] = {
        {DIK_ESCAPE, "ESC"},     {DIK_BACK, "BACK"},       {DIK_TAB, "TAB"},       {DIK_RETURN, "ENTER"},
        {DIK_RCONTROL, "RCTRL"}, {DIK_LCONTROL, "LCTRL"},  {DIK_LSHIFT, "LSHIFT"}, {DIK_RSHIFT, "RSHIFT"},
        {DIK_SPACE, "SPACE"},    {DIK_CAPITAL, "CAPSL"},   {DIK_SCROLL, "SCROLL"}, {DIK_LMENU, "LALT"},
        {DIK_RMENU, "RALT"},     {DIK_PAUSE, "PAUSE"},     {DIK_HOME, "HOME"},     {DIK_PRIOR, "PGUP"},
        {DIK_UP, "UP"},          {DIK_LEFT, "LEFT"},       {DIK_DOWN, "DOWN"},     {DIK_RIGHT, "RIGHT"},
        {DIK_DELETE, "DEL"},     {DIK_INSERT, "INS"},      {DIK_END, "END"},       {DIK_NEXT, "PGDWN"},
        {DIK_APPS, "MENU"},      {DIK_NUMPADCOMMA, "KP,"}, {DIK_DECIMAL, "KP."},   {DIK_NUMPADENTER, "KPEnter"},
        {DIK_NUMLOCK, "NumL"},   {DIK_NUMPAD0, "KP0"},     {DIK_NUMPAD1, "KP1"},   {DIK_NUMPAD2, "KP2"},
        {DIK_NUMPAD3, "KP3"},    {DIK_NUMPAD4, "KP4"},     {DIK_NUMPAD5, "KP5"},   {DIK_NUMPAD6, "KP6"},
        {DIK_NUMPAD7, "KP7"},    {DIK_NUMPAD8, "KP8"},     {DIK_NUMPAD9, "KP9"},   {DIK_F1, "F1"},
        {DIK_F2, "F2"},          {DIK_F3, "F3"},           {DIK_F4, "F4"},         {DIK_F5, "F5"},
        {DIK_F6, "F6"},          {DIK_F7, "F7"},           {DIK_F8, "F8"},         {DIK_F9, "F9"},
        {DIK_F10, "F10"},        {DIK_F11, "F11"},         {DIK_F12, "F12"},       {DIK_F13, "F13"},
        {DIK_F14, "F14"},        {DIK_F15, "F15"},
    };

    static Input *&instance;
};

extern Input *&dword_965DDC;

extern void Input_patch();
