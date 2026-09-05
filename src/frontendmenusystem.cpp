#include "frontendmenusystem.h"

#include "common.h"
#include "cursor.h"
#include "femenu.h"
#include "func_wrapper.h"
#include "game.h"
#include "game_settings.h"
#include "input.h"
#include "main_menu_credits.h"
#include "main_menu_keyboard.h"
#include "main_menu_legal.h"
#include "main_menu_load.h"
#include "main_menu_memcard_check.h"
#include "main_menu_options.h"
#include "main_menu_start.h"
#include "memory.h"
#include "mission_manager.h"
#include "movie_manager.h"
#include "ngl.h"
#include "ngl_scene.h"
#include "os_developer_options.h"
#include "panelfile.h"
#include "timer.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

VALIDATE_SIZE(FrontEndMenuSystem, 0x80);

static bool &already_drew_this_frame = var<bool>(0x0096B44A);

FrontEndMenuSystem::FrontEndMenuSystem() : FEMenuSystem(7, static_cast<font_index>(1))
{
    if constexpr (1) {
        this->field_50 = 0;
        this->field_51 = 0;
        this->field_52 = 0;

        static Var<bool> first_time_through{0x00937B78};
        static bool standalone_first_time_through = true;
        bool &is_first_time_through = STANDALONE_SYSTEM
            ? standalone_first_time_through
            : first_time_through();

        if (is_first_time_through) {
            this->field_4[this->m_count++] = new main_menu_legal{this, 320, 240};

            this->field_4[this->m_count++] = new main_menu_start{this, 320, 240};

            if constexpr (!STANDALONE_SYSTEM) {
                this->field_4[this->m_count++] = new main_menu_memcard_check{this, 320, 240};
                this->field_4[this->m_count++] = new main_menu_options{this, 320, 240};
                this->field_4[this->m_count++] = new main_menu_load{this, 320, 240};
                this->field_4[this->m_count++] = new main_menu_keyboard{this, 320, 240};
                this->field_4[this->m_count++] = new main_menu_credits{this, 320, 240};
            }

            is_first_time_through = false;
            this->field_50 = false;
        } else {
            this->field_50 = true;
        }

        this->field_34 = 0;
        this->field_51 = 0;
        this->field_52 = 0;
        this->field_7C = PanelFile::UnmashPanelFile("main_menu", static_cast<panel_layer>(7));
        for (int i = 0; i < this->m_count; ++i) {
            this->field_4[i]->Init();
        }

        static Var<bool> byte_96B44B{0x0096B44B};
        static bool standalone_byte_96B44B = false;
        const bool byte_96B44B_value = STANDALONE_SYSTEM
            ? standalone_byte_96B44B
            : byte_96B44B();

        auto v17 = !byte_96B44B_value;
        auto v18 = this->m_index;
        this->field_34 = v18;
        if (v17) {
            this->field_30 = 1;
            if (v18 != -1) {
                this->field_4[v18]->OnDeactivate(this->field_4[0]);
            }

            auto **v21 = this->field_4;
            this->m_index = 0;
            (*v21)->OnActivate();
            this->UpdateButtonDown();

        } else {
            this->field_30 = 3;
            if (v18 != -1) {
                this->field_4[v18]->OnDeactivate(*(this->field_4 + 1));
            }

            auto **v19 = this->field_4;
            this->m_index = 1;
            v19[1]->OnActivate();
            this->UpdateButtonDown();
            this->sub_60C240();
        }

    } else {
        THISCALL(0x00648580, this);
    }
}

void FrontEndMenuSystem::sub_60C240()
{
    if constexpr (STANDALONE_SYSTEM)
        field_38 = 0;
    else
        THISCALL(0x0060C240, this);
}

bool FrontEndMenuSystem::WaitForMemCheck()
{
    return this->field_30 != 10 && !os_developer_options::instance->os_developer_options::get_flag(
                                       static_cast<os_developer_options::flags_t>(66));
}

bool FrontEndMenuSystem::sub_60C230()
{
    return this->field_30 != 10;
}

bool sub_5A6880(const char *a1, void *a2)
{
    return CDECL_CALL(0x005A6880, a1, a2);
}

void sub_582960(bool a1)
{
    if (a1) {
        dword_965DDC->sub_821490(TRUE);

        static char byte_965C68[10][37]{};
        static_assert(std::size(byte_965C68[0]) == 37, "");
        static_assert(std::size(byte_965C68) == 10, "");

        int v1 = 0;
        for (auto &v2 : byte_965C68) {
            if (strcmp(v2, dword_965DDC->sub_81FD40(v1)) != 0) {
                EnterCriticalSection(&g_CriticalSection);
                byte_965950 = true;
                LeaveCriticalSection(&g_CriticalSection);
            }
            ++v1;
        }

        if (byte_965950) {
            int v3 = 0;
            for (auto &v4 : byte_965C68) {
                strcpy(v4, dword_965DDC->sub_81FD40(v3));
                ++v3;
            }
        }
    }
}

void __stdcall StartAddress([[maybe_unused]] LPVOID lpThreadParameter)
{
    for (auto i = WaitForSingleObject(hEvent, 0x3E8u); i != 0; i = WaitForSingleObject(hEvent, 0x3E8u)) {
        if (i == 258) {
            sub_582960(1);
        }
    }

    ExitThread(0);
}

void sub_582AD0()
{
    if (hObject == nullptr) {
        if (dword_965DDC == nullptr) {
            dword_965DDC = new Input{};
            dword_965DDC->initialize(g_appHwnd);
        }

        dword_965DDC->sub_821490(FALSE);
        sub_582960(true);
        byte_965950 = false;
        hEvent = CreateEventA(nullptr, FALSE, FALSE, "END EVENT");
        hObject = CreateThread(nullptr, 0, bit_cast<LPTHREAD_START_ROUTINE>(&StartAddress), nullptr, 0, nullptr);
        SetThreadPriority(hObject, 0);
    }
}

void sub_582BB0()
{
    if constexpr (0) {
        EnterCriticalSection(&g_CriticalSection);
        auto v0 = byte_965950;
        byte_965950 = false;
        LeaveCriticalSection(&g_CriticalSection);
        if (v0) {
            SetEvent(hEvent);
            WaitForSingleObject(hObject, 0xFFFFFFFF);
            CloseHandle(hObject);
            CloseHandle(hEvent);
            hObject = 0;
            hEvent = 0;
            sub_5A6880("GAMEPAD_CONNECTED", nullptr);
            Input::instance->sub_821490(TRUE);
            sub_5828B0();
            sub_582AD0();
        }
    } else {
        CDECL_CALL(0x00582BB0);
    }
}

void FrontEndMenuSystem::sub_619030(bool a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        static DWORD previous_tick = GetTickCount();
        const DWORD current_tick = GetTickCount();
        float delta_time = static_cast<float>(current_tick - previous_tick) * 0.001f;
        previous_tick = current_tick;
        if (delta_time <= 0.0f)
            delta_time = 0.000001f;

        UpdateButtonPresses();
        if (m_index >= 0) {
            auto *menu = field_4[m_index];
            if (menu != nullptr) {
                if (menu->m_vtbl == 0x00894598)
                    static_cast<main_menu_legal *>(menu)->Update(delta_time);
                else if (menu->m_vtbl == 0x00894648)
                    static_cast<main_menu_start *>(menu)->Update(delta_time);
                else
                    menu->Update(delta_time);
            }
        }

        if (field_7C != nullptr)
            field_7C->Update(delta_time);

        if (!a2) {
            nglListInit();
            nglSetClearFlags(7);
            nglSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            nglSetAspectRatio(1.0f);
            nglSetOrthoMatrix(1000.0f, 10000.0f);
            auto *menu = m_index >= 0 ? field_4[m_index] : nullptr;
            if (menu != nullptr && menu->m_vtbl == 0x00894598)
                static_cast<main_menu_legal *>(menu)->Draw();
            else if (field_7C != nullptr)
                field_7C->Draw();
        }
    } else {
        THISCALL(0x00619030, this, a2);
    }
}

void FrontEndMenuSystem::MakeActive(int a2)
{
    auto idx = this->m_index;
    this->field_34 = idx;
    if (idx != -1) {
        FEMenu *v4;
        if (a2 < 0) {
            v4 = nullptr;
        } else {
            v4 = this->field_4[a2];
        }

        auto *v8 = v4;
        auto *v5 = this->field_4[idx];

        v5->OnDeactivate(v8);
    }

    this->m_index = a2;

    if (a2 >= 0) {
        auto *v6 = this->field_4[a2];

        v6->OnActivate();
    }

    this->UpdateButtonDown();
}

void FrontEndMenuSystem::_LoadAll()
{
    TRACE("FrontEndMenuSystem::LoadAll");

    this->field_7C = PanelFile::UnmashPanelFile("main_menu", static_cast<panel_layer>(7));
}

void FrontEndMenuSystem::RenderLoadMeter(bool a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        sub_619030(a1);
    } else if (!os_developer_options::instance->get_flag(mString{"NO_LOAD_SCREEN"})) {
        THISCALL(0x00619230, this, a1);
    }
}

void FrontEndMenuSystem::GoNextState()
{
    if (this->field_30 == 10) {
        return;
    }

    int v3;
    while (2) {
        auto v2 = this->field_30;
        switch (v2) {
        case 5:
            this->field_50 = 1;
            this->field_30 = 6;
            v3 = this->field_30;
            break;
        case 0:
            this->field_50 = 1;
            this->field_30 = 1;
            v3 = 1;
            break;
        case 4:
            this->field_52 = 1;
            this->field_30 = 5;
            v3 = 5;
            break;
        case 6: {
            switch (this->field_4[this->m_index][5].field_28) {
            case 0: {
                g_game_ptr->gamefile->load_most_recent_game();
                this->BringUpDialogBox(10, fe_state{10}, fe_state{10});
                v3 = this->field_30;
                break;
            }
            case 1:
                this->field_30 = 7;
                v3 = this->field_30;
                break;
            case 2:
                this->field_30 = 9;
                v3 = this->field_30;
                break;
            case 3:
                this->field_30 = 8;
                v3 = this->field_30;
                break;
            case 4:
                sub_5A6D70();
                v3 = this->field_30;
                break;
            case 5: {
                bExit = true;
                v3 = this->field_30;
                break;
            }
            default:
                v3 = this->field_30;
                break;
            }

            break;
        }
        case 7: {
            this->field_30 = 10;
            v3 = 10;
            break;
        }
        case 11:
            this->field_30 =
                (bit_cast<main_menu_memcard_check *>(this->field_4[2])->field_100 ? this->field_58 : this->field_54);

            if (this->field_5C.size() > 0) {
                int v5;
                this->field_54 = *(uint32_t *)*this->field_5C.sub_64A090(&v5);

                int v6;
                this->field_58 = *(uint32_t *)*this->field_6C.sub_64A090(&v6);
                this->field_5C.sub_64A2B0();
                this->field_6C.sub_64A2B0();
            }

            break;
        case 3:
            this->field_30 = 5;
            v3 = 5;
            break;
        default:
            this->field_30 = v2 + 1;
            break;
        }

        v3 = this->field_30;
        switch (v3) {
        case 0:
        case 11: {
            if (this->field_34 != 2) {
                this->MakeActive(2);
                break;
            }

            auto v4 = this->field_30;
            this->sub_6342D0();
            this->field_30 = v4;
            break;
        }
        case 1: {
            this->MakeActive(false);
            break;
        }
        case 2: {
            if constexpr (STANDALONE_SYSTEM) {
                movie_manager::load_and_play_movie(
                    "mlogonosound", "Marvel_Logo", false);
                movie_manager::load_and_play_movie(
                    "ATVI spin logo 640 none", "Activision", false);
                movie_manager::load_and_play_movie(
                    "Treyarch_USM_logo", "TREYARCH_LOGO", false);
                movie_manager::load_and_play_movie("beenox_short", nullptr, false);
                this->field_30 = 3;
                this->MakeActive(1);
                this->sub_60C240();
                return;
            } else {
                if (nglCurScene != nullptr) {
                    --this->field_30;
                } else {
                    if (!movie_manager::load_and_play_movie("mlogonosound", "Marvel_Logo", false) &&
                        !movie_manager::load_and_play_movie("ATVI spin logo 640 none", "Activision", false) &&
                        !movie_manager::load_and_play_movie("Treyarch_USM_logo", "TREYARCH_LOGO", false)) {
                        movie_manager::load_and_play_movie("beenox_short", nullptr, false);
                    }

                    if (this->field_30 != 10) {
                        continue;
                    }
                }
            }

            break;
        }
        case 3: {
            this->MakeActive(true);
            this->sub_60C240();
            break;
        }
        case 4:
            if (this->m_index != 1) {
                this->MakeActive(true);
            }

            this->field_52 = true;
            break;
        case 5: {
            this->MakeActive(2);
            break;
        }
        case 6:
            this->MakeActive(3);
            break;
        case 7:
            this->MakeActive(5);
            break;
        case 8:
            this->MakeActive(6);
            break;
        case 9:
            this->MakeActive(4);
            break;
        case 10: {
            if (this->m_index != 5) {
                mission_manager::s_inst->sub_5BACA0(0.0);
            }

            this->sub_60C290();
            break;
        }
        default:
            return;
        }
        break;
    }
}

void FrontEndMenuSystem::BringUpDialogBox(int a2, FrontEndMenuSystem::fe_state a3, FrontEndMenuSystem::fe_state a4)
{
    THISCALL(0x00634300, this, a2, a3, a4);
}

void FrontEndMenuSystem::sub_60C290()
{
    THISCALL(0x0060C290, this);
}

void FrontEndMenuSystem::sub_6342D0()
{
    THISCALL(0x006342D0, this);
}

void FrontEndMenuSystem_patch()
{
    {
        FUNC_ADDRESS(address, &FrontEndMenuSystem::_LoadAll);
        set_vfunc(0x0089A2B0, address);
    }

    return;

    FUNC_ADDRESS(address, &FrontEndMenuSystem::MakeActive);
    set_vfunc(0x0089A27C, address);

    {
        FUNC_ADDRESS(address, &FrontEndMenuSystem::GoNextState);
        SET_JUMP(0x00635BC0, address);
    }
}
