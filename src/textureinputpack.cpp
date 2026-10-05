#include "textureinputpack.h"

#include "common.h"
#include "float.hpp"
#include "func_wrapper.h"
#include "input.h"
#include "ngl_dx_texture.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <d3d9.h>


#include <d3dx9tex.h>
#include <algorithm>
#include <array>
#include <cassert>
#include "femanager.h"
#include "fileusm.h"
#include "game.h"
#include "ngl.h"
#include "ngl_font.h"
#include "sound_instance_id.h"
#include "timer.h"

#if STANDALONE_SYSTEM
void sub_582AD0();
namespace {
struct dialog_vertex {
    float x, y, z, rhw;
    uint32_t color;
    float u, v;
};

struct dialog_texture {
    IDirect3DTexture9 *value{};
    explicit dialog_texture(const wchar_t *path)
    {
        const auto result = D3DXCreateTextureFromFileExW(
            g_Direct3DDevice, path, 0, 0, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, 3, 3, 0, nullptr, nullptr, &value);
        assert(SUCCEEDED(result));
    }
    ~dialog_texture()
    {
        value->lpVtbl->Release(value);
    }
    dialog_texture(const dialog_texture &) = delete;
    dialog_texture &operator=(const dialog_texture &) = delete;
};

void dialog_quad(float x, float y, float right, float bottom, float u0, float v0, float u1, float v1, uint32_t color)
{
    const dialog_vertex vertices[] = {{x, y, 0.99f, 1.0f, color, u0, v0},
                                      {x, bottom, 0.99f, 1.0f, color, u0, v1},
                                      {right, y, 0.99f, 1.0f, color, u1, v0},
                                      {right, bottom, 0.99f, 1.0f, color, u1, v1}};
    auto *device = g_Direct3DDevice;
    device->lpVtbl->DrawPrimitiveUP(device, D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(dialog_vertex));
}

int dialog_text_width(nglFont *font, const char *text, float scale)
{
    int widest = 0, width = 0;
    for (auto *at = reinterpret_cast<const unsigned char *>(text); *at; ++at) {
        if (*at == '\n') {
            widest = std::max(widest, width);
            width = 0;
        } else {
            width = static_cast<int>(width + font->GetFontCellWidth(*at) * scale);
        }
    }
    return std::max(widest, width);
}

struct native_dialog {
    const native_dialog *parent;
    const char *message;
    bool confirmation;
    nglFont *font;
    dialog_texture panel{L"data\\packs\\igq_bk.dat"};
    dialog_texture button{L"data\\packs\\igq_yes_no.dat"};
    dialog_texture cursor{L"Data\\ump.dat"};
    int x, y, width, height, button_y;
    float scale_x, scale_y;
    int selected, pressed{-1};
    POINT mouse{};
    bool mouse_visible{};

    native_dialog(const char *key, bool confirm, const native_dialog *owner)
        : parent(owner), message(get_msg(g_fileUSM, key)), confirmation(confirm),
          font(g_femanager.GetFont(static_cast<font_index>(1))), selected(confirm ? 1 : 0)
    {
        width = dialog_text_width(font, message, 1.0f) + 5;
        const int lines = static_cast<int>(std::count(message, message + std::strlen(message), '\n')) + 1;
        height = 30 * (lines + 1);
        x = 320 - width / 2;
        y = 240 - height / 2;
        button_y = y + 30 * lines;
        scale_x = g_cx / 640.0f;
        scale_y = g_cy / 480.0f;
    }

    void quad(IDirect3DTexture9 *texture, float left, float top, float right, float bottom, float v0 = 0.0f,
              float v1 = 1.0f) const
    {
        auto *device = g_Direct3DDevice;
        device->lpVtbl->SetTexture(device, 0, reinterpret_cast<IDirect3DBaseTexture9 *>(texture));
        dialog_quad(left * scale_x, top * scale_y, right * scale_x, bottom * scale_y, 0.0f, v0, 1.0f, v1, 0xffffffffu);
    }

    void text(const char *value, int left, int top, int right, bool centered, float scale, uint32_t color) const
    {
        auto *device = g_Direct3DDevice;
        device->lpVtbl->SetTexture(device, 0, reinterpret_cast<IDirect3DBaseTexture9 *>(font->field_24->DXTexture));
        const char *line = value;
        while (*line) {
            const char *end = line;
            int line_width = 0;
            while (*end && *end != '\n') {
                line_width = static_cast<int>(line_width + font->GetFontCellWidth(static_cast<uint8_t>(*end)) * scale);
                ++end;
            }
            float pen = static_cast<float>(centered ? left + (right - left) / 2 - line_width / 2 : left);
            for (auto *at = line; at != end; ++at) {
                float origin[2], size[2], uv[2], extent[2];
                font->sub_77E2F0(static_cast<uint8_t>(*at), origin, size, uv, extent, scale, scale);
                dialog_quad((pen + origin[0]) * scale_x,
                            (top + origin[1]) * scale_y,
                            (pen + origin[0] + size[0]) * scale_x,
                            (top + origin[1] + size[1]) * scale_y,
                            uv[0],
                            uv[1],
                            uv[0] + extent[0],
                            uv[1] + extent[1],
                            color);
                pen += font->GetFontCellWidth(static_cast<uint8_t>(*at)) * scale;
            }
            if (!*end)
                break;
            line = end + 1;
            top += static_cast<int>(22.0f * scale);
        }
    }

    void draw() const
    {
        if (parent)
            parent->draw();
        else
            quad(nglGetBackBufferTex()->DXTexture, 0, 0, 640, 480);
        quad(panel.value, x - 10, y - 10, x + width + 9, y + height + 9);
        text(message, x, y, x + width - 1, false, 1.0f, confirmation ? 0xffdcdcdcu : 0xffdcdedcu);
        const int count = confirmation ? 2 : 1;
        for (int index = 0; index < count; ++index) {
            const int left = x + index * (width / count);
            const int right = left + width / count - 1;
            quad(button.value, left, button_y, right, button_y + 29, 0.0f, 0.2f);
            if (selected == index)
                quad(button.value, left, button_y, right, button_y + 29, 0.2f, 0.4f);
            const char *key = confirmation ? (index == 0 ? "YES" : "NO") : "OK";
            text(get_msg(g_fileUSM, key),
                 left,
                 button_y,
                 right,
                 true,
                 1.25f,
                 selected == index ? 0xffe6d03fu : 0xffe6823fu);
        }
        if (mouse_visible)
            quad(cursor.value, mouse.x / scale_x, mouse.y / scale_y, mouse.x / scale_x + 47, mouse.y / scale_y + 47);
    }

    int hit(POINT point) const
    {
        const int count = confirmation ? 2 : 1;
        for (int index = 0; index < count; ++index) {
            RECT bounds{static_cast<LONG>((x + index * (width / count)) * scale_x),
                        static_cast<LONG>(button_y * scale_y),
                        static_cast<LONG>((x + (index + 1) * (width / count) - 1) * scale_x),
                        static_cast<LONG>((button_y + 29) * scale_y)};
            if (PtInRect(&bounds, point))
                return index;
        }
        return -1;
    }

    void sound(const char *key) const
    {
        static_cast<void>(sub_60B960(string_hash{key}, 1.0f, 1.0f));
    }
};

bool run_native_dialog(const char *key, bool confirmation, const native_dialog *parent)
{
    auto *device = g_Direct3DDevice;
    IDirect3DStateBlock9 *saved_state = nullptr;
    auto result = device->lpVtbl->CreateStateBlock(device, D3DSBT_ALL, &saved_state);
    assert(SUCCEEDED(result));
    saved_state->lpVtbl->Capture(saved_state);
    auto *saved_input = Input::instance->field_129D8[0];
    Input::instance->sub_8203F0(0, g_inputSettingsMenu);
    auto &modal_active = var<bool>(0x00965C22);
    const bool previous_modal = modal_active;
    if (confirmation)
        modal_active = true;
    native_dialog dialog{key, confirmation, parent};
    IDirect3DSurface9 *backbuffer = nullptr;
    device->lpVtbl->GetBackBuffer(device, 0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);
    device->lpVtbl->SetRenderTarget(device, 0, backbuffer);
    backbuffer->lpVtbl->Release(backbuffer);
    constexpr std::array<InputAction, 6> actions{static_cast<InputAction>(16),
                                                 static_cast<InputAction>(17),
                                                 static_cast<InputAction>(18),
                                                 static_cast<InputAction>(19),
                                                 static_cast<InputAction>(4),
                                                 static_cast<InputAction>(7)};
    std::array<int, 6> repeat{};
    int answer = -1;
    DWORD previous_tick = GetTickCount();
    auto accept = [&] {
        answer = !confirmation || dialog.selected == 0;
        dialog.sound(answer ? "FE_PS_ACCEPT" : "FE_PS_BACK");
    };
    while (answer == -1 && byte_965BF9) {
        MSG message;
        while (PeekMessageA(&message, g_appHwnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessageA(&message);
            if (message.message == WM_MOUSEMOVE) {
                dialog.mouse = {static_cast<short>(LOWORD(message.lParam)), static_cast<short>(HIWORD(message.lParam))};
                dialog.mouse_visible = true;
                const int hovered = dialog.hit(dialog.mouse);
                if (hovered != -1)
                    dialog.selected = hovered;
            } else if (message.message == WM_LBUTTONDOWN) {
                const POINT point{static_cast<short>(LOWORD(message.lParam)),
                                  static_cast<short>(HIWORD(message.lParam))};
                dialog.pressed = dialog.hit(point);
            } else if (message.message == WM_LBUTTONUP) {
                const POINT point{static_cast<short>(LOWORD(message.lParam)),
                                  static_cast<short>(HIWORD(message.lParam))};
                if (dialog.pressed != -1 && dialog.hit(point) == dialog.pressed) {
                    dialog.selected = dialog.pressed;
                    accept();
                }
                dialog.pressed = -1;
            } else if (message.message == WM_RBUTTONUP) {
                answer = 0;
                if (confirmation)
                    dialog.sound("FE_PS_BACK");
            }
        }
        auto &quit_count = dword_922908;
        if (quit_count > 0)
            --quit_count;
        else if (quit_count == 0) {
            quit_count = -1;
            if (run_native_dialog("CONFIRMQUIT_MSG", true, &dialog)) {
                ClipCursor(nullptr);
                bExit = true;
                answer = 0;
            }
        }
        Input::instance->poll();
        for (size_t index = 0; index < actions.size(); ++index) {
            const bool down = g_inputSettingsMenu->field_18.get_state(actions[index]) > 0.5f;
            if (!down) {
                if (repeat[index] && index == 4)
                    accept();
                repeat[index] = 0;
            } else if (repeat[index] == 0) {
                repeat[index] = 10;
                dialog.mouse_visible = false;
                if ((index == 2 || index == 3) && confirmation) {
                    dialog.selected = 1 - dialog.selected;
                    dialog.sound("FE_WB_LRScroll");
                } else if (index == 5 && confirmation) {
                    answer = 0;
                    dialog.sound("FE_PS_BACK");
                }
            } else if (index < 4) {
                --repeat[index];
            }
        }
        const DWORD tick = GetTickCount();
        Float elapsed{(tick - previous_tick) * 0.001f};
        previous_tick = tick;
        g_game_ptr->sub_559F50(&elapsed);
        EnterCriticalSection(&g_CriticalSection);
        const bool gamepad_changed = byte_965950;
        byte_965950 = false;
        LeaveCriticalSection(&g_CriticalSection);
        if (gamepad_changed) {
            SetEvent(hEvent);
            WaitForSingleObject(hObject, INFINITE);
            CloseHandle(hObject);
            CloseHandle(hEvent);
            hObject = nullptr;
            hEvent = nullptr;
            run_native_dialog("GAMEPAD_CONNECTED", false, &dialog);
            Input::instance->sub_821490(true);
            sub_5828B0();
            sub_582AD0();
        }
        device->lpVtbl->Clear(device, 0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
        device->lpVtbl->BeginScene(device);
        device->lpVtbl->SetRenderState(device, D3DRS_FOGENABLE, false);
        device->lpVtbl->SetRenderState(device, D3DRS_ZENABLE, false);
        device->lpVtbl->SetRenderState(device, D3DRS_ALPHATESTENABLE, true);
        device->lpVtbl->SetRenderState(device, D3DRS_ALPHABLENDENABLE, true);
        device->lpVtbl->SetRenderState(device, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        device->lpVtbl->SetRenderState(device, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        device->lpVtbl->SetRenderState(device, D3DRS_CULLMODE, D3DCULL_NONE);
        device->lpVtbl->SetVertexShader(device, nullptr);
        device->lpVtbl->SetPixelShader(device, nullptr);
        device->lpVtbl->SetFVF(device, D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
        device->lpVtbl->SetTextureStageState(device, 0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        device->lpVtbl->SetTextureStageState(device, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        device->lpVtbl->SetTextureStageState(device, 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        device->lpVtbl->SetTextureStageState(device, 0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        device->lpVtbl->SetTextureStageState(device, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        device->lpVtbl->SetTextureStageState(device, 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        device->lpVtbl->SetTextureStageState(device, 1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        device->lpVtbl->SetTextureStageState(device, 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        device->lpVtbl->SetSamplerState(device, 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        device->lpVtbl->SetSamplerState(device, 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        device->lpVtbl->SetSamplerState(device, 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        device->lpVtbl->SetSamplerState(device, 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        dialog.draw();
        device->lpVtbl->EndScene(device);
        device->lpVtbl->Present(device, nullptr, nullptr, nullptr, nullptr);
    }
    g_timer->sub_582180();
    Input::instance->sub_8203F0(0, saved_input);
    modal_active = previous_modal;
    saved_state->lpVtbl->Apply(saved_state);
    saved_state->lpVtbl->Release(saved_state);
    return answer == 1;
}
}

bool show_native_confirmation_dialog(const char *message_key)
{
    return run_native_dialog(message_key, true, nullptr);
}
#endif

VALIDATE_SIZE(TexturePackBase, 0x6C);

VALIDATE_OFFSET(TextureInputPack, field_9C, 0x9C);
VALIDATE_OFFSET(TextureInputPack, field_3C0, 0x3C0);
VALIDATE_OFFSET(TextureInputPack, field_414, 0x414);
VALIDATE_OFFSET(TextureInputPack, field_5AC, 0x5AC);

TextureInputPack::TextureInputPack() {}

void __fastcall sub_585BE0(void *self, void *, void *a2, int a3, int a4, int a5, int a6, Float a7)
{
    THISCALL(0x00585BE0, self, a2, a3, a4, a5, a6, a7);
}

void TextureInputPack::sub_5871D0()
{
    if constexpr (0) {
        if (g_distance_clipping_enabled) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_FOGENABLE, false);
        }

        nglSetTextureStageState(0, D3DTSS_COLOROP, 2u);
        nglSetTextureStageState(0, D3DTSS_COLORARG1, 2u);
        nglSetTextureStageState(0, D3DTSS_ALPHAOP, 2u);
        nglSetTextureStageState(0, D3DTSS_ALPHAARG1, 2u);

        auto v2 = this->field_4;
        auto v3 = this->field_8;
        auto v4 = this->field_C - v2 + 1;
        auto v49 = this->field_98 - this->field_94;
        [[maybe_unused]] auto v48 = v4;

        int v54[4];

        int v5 = 0;
        auto *v46 = &this->field_3C0;
        do {
            auto *v6 = v46;
            v54[v5++] = (v4 * *v46);
            v46 = v6 + 1;
        } while (v5 < 4);

        int v7 = 0;
        if (this->field_9C + 1 > 0) {
            char *v46 = this->field_A0;
            do {
                auto v8 = this->field_3D0;
                auto v48 = v2 + v54[v7];

                auto v9 = this->field_3E0;
                auto v38 = this->field_3EC;
                auto v10 = v8 + v3 - 1;
                auto *v11 = this->field_1C;

                void(__fastcall * func4)(void *, void *, void *, int, int, int, int, color32, int, Float, Float, int) =
                    CAST(func4, get_vfunc(v11->m_vtbl, 0x4));

                func4(v11, nullptr, v46, v2, v3, v48 - 1, v10, v38, v9, Float{1.0}, Float{1.0}, 0);

                v2 = v48;
                ++v7;

                v46 += 50;
            } while (v7 < (this->field_9C + 1));
        }

        auto v13 = this->field_3D0;
        auto v14 = this->field_94;
        auto *v15 = (int *)this->field_88;
        auto v16 = v13 + v3;
        auto v47 = v16;
        auto *v44 = v15;
        if (v14 > 0) {
            do {
                --v14;
                v44 = (int *)v44[61];
            } while (v14 != 0);
        }

        auto v17 = (this->field_10 - v13 - this->field_8 + 1) / this->field_3D4;

        int v18 = 0;
        int v45 = 0;
        for (auto i = v17; v18 < v17; v45 = v18) {
            if (v18 >= v49) {
                break;
            }

            auto v19 = this->field_9C;
            char *v46 = (char *)this->field_4;
            int v20 = 0;
            int v43 = 0;
            if (v19 + 1 > 0) {
                while (1) {
                    auto v21 = this->field_3D8;
                    auto v22 = v21 + v16;
                    auto v23 = v16 - v21 + this->field_3D4 - 1;
                    auto *v24 = &v46[v21];
                    auto v52 = (int)&v46[v54[v20] - v21 - 1];
                    if (v20 <= 0) {
                        void(__fastcall *
                             func4)(void *, void *, char *, int, int, int, int, color32, int, Float, Float, int) =
                            CAST(func4, get_vfunc(this->field_1C->m_vtbl, 0x4));

                        func4(this->field_1C,
                              nullptr,
                              (char *)v44 + 8,
                              (int)v46 + v21,
                              v22,
                              (int)v52,
                              v23,
                              this->field_3F0,
                              this->field_3E4,
                              Float{1.0},
                              Float{1.0},
                              0);
                    } else if (v45 + this->field_94 == this->field_8C && v20 == this->field_90 + 1) {
                        int v39;
                        color32 v25;
                        if (this->field_410 == v20 && this->field_414 == v45) {
                            v39 = this->field_3E8;
                            v25 = this->field_3FC;
                        } else {
                            v39 = this->field_3E8;
                            v25 = this->field_3F4;
                        }

                        void(__fastcall *
                             func4)(void *, void *, const char *, int, int, int, int, color32, int, Float, Float, int) =
                            CAST(func4, get_vfunc(this->field_1C->m_vtbl, 0x4));
                        func4(this->field_1C, nullptr, "?????", (int)v24, v22, (int)v52, v23, v25, v39, 1.0, 1.0, 0);
                    } else {
                        auto *v26 = &v44[6 * this->field_40C + 3 * this->field_40C + 3 * v20];
                        auto *v27 = this->field_418->get_string((InputType)v26[49], v26[50]);
                        auto v48 = (int)v27;

                        char v42;
                        unsigned int v50;
                        if (v27 != nullptr && (v42 = (int)(v27 + 1), (v50 = strlen(v27)) != 0)) {
                            v42 = ' ';
                            v50 = (unsigned int)(v27 + 1);
                            if (strlen(v27) == 1) {
                                auto v28 = *v27;
                                switch (v28) {
                                case -32:
                                case -28:
                                case -25:
                                case -24:
                                case -23:
                                case -20:
                                case -15:
                                case -14:
                                case -10:
                                case -7:
                                case -4:
                                    v42 = v28 - 32;
                                    break;
                                default:
                                    break;
                                }
                            }

                            int v29 = v44[6 * this->field_40C + 49 + 3 * this->field_40C + 3 * v43];
                            if (v29 != 1 && v29 != 2) {
                                auto v30 = (int)v24;
                                auto v31 = (int)&v24[v23 - v22];
                                [[maybe_unused]] auto *v24 = (char *)(v31 + this->field_3D8);

                                switch (v29) {
                                case 3:
                                    sub_585BE0(this->field_1C, nullptr, this->field_78, v30, v22, v31, v23, 0.2);
                                    break;
                                case 4:
                                    sub_585BE0(this->field_1C, nullptr, this->field_7C, v30, v22, v31, v23, 0.2);
                                    break;
                                case 5:
                                    sub_585BE0(this->field_1C, nullptr, this->field_80, v30, v22, v31, v23, 0.2);
                                    break;
                                case 6:
                                    sub_585BE0(this->field_1C, nullptr, this->field_84, v30, v22, v31, v23, 0.2);
                                    break;
                                default:
                                    break;
                                }
                            }

                            int v40;
                            color32 v32;
                            if (this->field_410 == v43 && this->field_414 == v45 && byte_965C20) {
                                v40 = this->field_3E8;
                                v32 = this->field_3FC;
                            } else {
                                v40 = this->field_3E8;
                                v32 = this->field_3F4;
                            }

                            auto *v33 = this->field_1C;
                            void(__fastcall * func4)(
                                void *, void *, const char *, int, int, int, int, color32, int, Float, Float, int) =
                                CAST(func4, get_vfunc(v33->m_vtbl, 0x4));
                            if (v42 == ' ') {
                                func4(v33, nullptr, (char *)v48, (int)v24, v22, (int)v52, v23, v32, v40, 1.0, 1.0, 0);
                            } else {
                                func4(v33, nullptr, (char *)&v42, (int)v24, v22, (int)v52, v23, v32, v40, 1.0, 1.0, 0);
                            }
                        } else {
                            int v41;
                            color32 v35;
                            if (this->field_410 == v43 && this->field_414 == v45 && byte_965C20) {
                                v41 = this->field_3E8;
                                v35 = this->field_3FC;
                            } else {
                                v41 = this->field_3E8;
                                v35 = this->field_3F8;
                            }

                            void(__fastcall * func4)(
                                void *, void *, const char *, int, int, int, int, color32, int, Float, Float, int) =
                                CAST(func4, get_vfunc(this->field_1C->m_vtbl, 0x4));
                            func4(this->field_1C,
                                  nullptr,
                                  this->field_5AC,
                                  (int)v24,
                                  v22,
                                  (int)v52,
                                  v23,
                                  v35,
                                  v41,
                                  1.0,
                                  1.0,
                                  0);
                        }
                    }

                    v16 = v47;
                    auto v36 = v43 + 1 < this->field_9C + 1;
                    v46 += v54[v43++];
                    if (!v36) {
                        break;
                    }

                    v20 = v43;
                }
                v18 = v45;
                v17 = i;
            }

            auto *v37 = (int *)v44[61];
            v16 += this->field_3D4;
            ++v18;
            v47 = v16;
            v44 = v37;
        }

        if (g_distance_clipping_enabled) {
            IDirect3DDevice9_SetRenderState(g_Direct3DDevice, D3DRS_FOGENABLE, true);
        }

    } else {
        THISCALL(0x005871D0, this);
    }
}

void __fastcall sub_5B24F0(void *self, void *, const char *a2, int a3, int a4, int a5, int a6, int a7, int a8, Float a9,
                           Float a10, int a11)
{
    THISCALL(0x005B24F0, self, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11);
}

void TextureInputPack_patch()
{
    set_vfunc(0x0088EBB0, sub_5B24F0);

    {
        FUNC_ADDRESS(address, &TextureInputPack::sub_5871D0);
        set_vfunc(0x0088E904, address);
    }
}
