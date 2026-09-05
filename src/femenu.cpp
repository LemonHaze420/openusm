#include "femenu.h"

#include "common.h"
#include "config.h"
#include "femenuentry.h"
#include "femenusystem.h"
#include "fetext.h"
#include "input_mgr.h"
#include "memory.h"
#include "fe_dialog_text.h"
#include "main_menu_keyboard.h"
#include "main_menu_load.h"
#include "main_menu_legal.h"
#include "main_menu_memcard_check.h"
#include "main_menu_options.h"
#include "main_menu_start.h"
#include "pause_menu_controller.h"
#include "pause_menu_message_log.h"
#include "pause_menu_options_display.h"
#include "pause_menu_root.h"
#include "pause_menu_save_load_display.h"
#include "pause_menu_status.h"
#include "pause_menu_transition.h"
#include "utility.h"
#include "variables.h"
#include "vtbl.h"

#include <cstdio>
#include <cstdlib>

VALIDATE_SIZE(FEMenu, 44u);

FEMenu::FEMenu(FEMenuSystem *a2, uint32_t a3, int a4, int a5, int16_t a6, int16_t a7)
{
    this->m_vtbl = 0x00893C88;

    this->field_4 = a3 == 0 ? nullptr : CAST(this->field_4, ::operator new(4 * a3));
    for (uint32_t i = 0; i < a3; ++i) {
        this->field_4[i] = nullptr;
    }

    this->field_28 = static_cast<decltype(this->field_28)>(a7 & 0xFDFF);
    this->field_C = a4;
    this->field_10 = a5;
    this->field_20 = 0;
    this->highlighted = -1;
    this->field_8 = a2;
    this->num_entries = static_cast<decltype(this->num_entries)>(a3);
    this->field_14 = 28;
    this->field_18 = 0;
    this->field_26 = a6;
    if (a2 != nullptr) {
        this->field_2B = a2->GetDefaultColorScheme();
    } else {
        this->field_2B = 0;
    }
}

void *FEMenu::operator new(size_t size)
{
    auto *mem = mem_alloc(size);
    return mem;
}

void FEMenu::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}

void FEMenu::AddEntry(int a2, FEText *a3, bool a4)
{
    auto *mem = mem_alloc(sizeof(FEMenuEntry));

    this->field_4[a2] = new (mem) FEMenuEntry{this, a3, a4};
}

void FEMenu::AddEntry(int a2, global_text_enum a3)
{
    auto *mem = mem_alloc(sizeof(FEMenuEntry));

    this->field_4[a2] = new (mem) FEMenuEntry{a3, this, false, static_cast<font_index>(6), 1};
}

void FEMenu::Init()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (m_vtbl != 0x00893C88) {
            switch (m_vtbl) {
            case 0x00894598:
                return;
            case 0x00895910:
                static_cast<main_menu_memcard_check *>(this)->_Init();
                return;
            case 0x008946F8:
                static_cast<main_menu_options *>(this)->_Init();
                return;
            case 0x00893E78:
                return;
            case 0x00894648:
                static_cast<main_menu_start *>(this)->_Init();
                return;
            case 0x00893F38:
            case 0x00893FE8:
                return;
            case 0x008947A8:
                static_cast<main_menu_load *>(this)->_Init();
                return;
            case 0x00894858:
                return;
            case 0x00895790:
                static_cast<main_menu_keyboard *>(this)->_Init();
                return;
            case 0x008940A0:
            case 0x00894150:
            case 0x00894200:
            case 0x008942B0:
            case 0x008943D8:
                return;
            default:
                sp_log("FEMenu::Init: unsupported standalone vtable 0x%08X", m_vtbl);
                std::fflush(nullptr);
                std::exit(3);
            }
        }

        if (num_entries > field_26) {
            field_28 |= 1;
        }

        int visible_entries;
        if ((field_28 & 1) != 0) {
            visible_entries = field_26;
        } else if ((field_28 & 0x40) != 0) {
            visible_entries = 0;
            for (int i = 0; i < num_entries; ++i) {
                if (!field_4[i]->GetDisable()) {
                    ++visible_entries;
                }
            }
        } else {
            visible_entries = num_entries;
        }

        field_18 = field_14 * (visible_entries - 1) / 2;
        if (num_entries > 0) {
            SetHigh(0, false);
        }
        field_28 |= 0x200;

        if ((field_28 & 1) != 0) {
            SetVis(0);
            return;
        }

        int visible_index = 0;
        for (int i = 0; i < num_entries; ++i) {
            if ((field_28 & 0x40) != 0 && field_4[i]->GetDisable()) {
                continue;
            }

            int y;
            if ((field_28 & 0x400) != 0) {
                y = field_10 + visible_index * field_14;
            } else if ((field_28 & 0x800) == 0) {
                y = field_10 + visible_index * field_14 - field_18;
            } else {
                y = field_10 + visible_index * field_14 - 2 * field_18;
            }

            field_4[i]->SetPos(field_C, y);
            ++visible_index;
        }
    } else {
        void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xC));
        func(this);
    }
}

void FEMenu::Load()
{
    if constexpr (STANDALONE_SYSTEM) {
        switch (m_vtbl) {
        case 0x00893C88:
            Update(0.0f);
            return;
        case 0x00893E78:
            static_cast<fe_dialog_text *>(this)->_Load();
            return;
        case 0x00893F38:
            static_cast<pause_menu_root *>(this)->_Load();
            return;
        case 0x00893FE8:
            static_cast<pause_menu_transition *>(this)->_Load();
            return;
        case 0x008940A0:
            static_cast<pause_menu_status *>(this)->_Load();
            return;
        case 0x00894150:
            static_cast<pause_menu_options_display *>(this)->_Load();
            return;
        case 0x00894200:
            static_cast<pause_menu_controller *>(this)->_Load();
            return;
        case 0x008942B0:
            static_cast<pause_menu_save_load_display *>(this)->_Load();
            return;
        case 0x008943D8:
            static_cast<pause_menu_message_log *>(this)->_Load();
            return;
        default:
            sp_log("FEMenu::Load: unsupported standalone vtable 0x%08X", m_vtbl);
            std::fflush(nullptr);
            std::exit(3);
        }
    } else {
        void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x10));
        func(this);
    }
}

void FEMenu::Draw()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x18));
    func(this);
}

void FEMenu::Update(Float a2)
{
    if ((128 & this->field_28) != 0 && this->field_2A != -1) {
        this->field_1C = this->field_1C - a2;
        if (this->field_1C <= 0.0f) {
            this->ButtonHeldAction();
            auto v4 = this->field_28;
            this->field_1C = 0.1;
            if ((v4 & 0x100) != 0) {
                this->field_1C = 0.010000001;
            }
        }
    }

    for (auto i = 0; i < this->num_entries; ++i) {
        auto *v6 = this->field_4[i];
        v6->Update(a2);
    }
}

void FEMenu::OnActivate()
{
    if constexpr (STANDALONE_SYSTEM) {
        switch (m_vtbl) {
        case 0x00894648:
            static_cast<main_menu_start *>(this)->_OnActivate();
            return;
        case 0x00894598:
            static_cast<main_menu_legal *>(this)->OnActivate();
            return;
        case 0x00893C88:
            return;
        default:
            sp_log("FEMenu::OnActivate: unsupported standalone vtable 0x%08X", m_vtbl);
            std::fflush(nullptr);
            std::exit(3);
        }
    } else {
        void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0x2C));
        func(this);
    }
}

void FEMenu::OnDeactivate(FEMenu *a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (m_vtbl == 0x00894598) {
            static_cast<main_menu_legal *>(this)->OnDeactivate();
            return;
        }
        if (m_vtbl == 0x00894648) {
            static_cast<main_menu_start *>(this)->_OnDeactivate();
            return;
        }
        if (m_vtbl == 0x00893C88) {
            return;
        }
    }
    void(__fastcall * func)(FEMenu *, void *, FEMenu *) = CAST(func, get_vfunc(m_vtbl, 0x30));
    func(this, nullptr, a2);
}

void FEMenu::OnSelect(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x34));
    func(this, nullptr, a2);
}

void FEMenu::OnStart(int controller)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (m_vtbl == 0x00894648) {
            static_cast<main_menu_start *>(this)->OnStart(controller);
        }
        return;
    }

    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x38));
    func(this, nullptr, controller);
}

void FEMenu::OnUp(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x3C));
    func(this, nullptr, a2);
}

void FEMenu::OnDown(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x40));
    func(this, nullptr, a2);
}

void FEMenu::OnLeft(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x44));
    func(this, nullptr, a2);
}

void FEMenu::OnRight(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x48));
    func(this, nullptr, a2);
}

void FEMenu::OnCross(int controller)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (m_vtbl == 0x00894648) {
            static_cast<main_menu_start *>(this)->OnCross(controller);
        }
        return;
    }

    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x4C));
    func(this, nullptr, controller);
}

void FEMenu::OnTriangle(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x50));

    func(this, nullptr, a2);
}

void FEMenu::OnSquare(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x54));

    func(this, nullptr, a2);
}

void FEMenu::OnCircle(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x58));

    func(this, nullptr, a2);
}

void FEMenu::OnL1(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x5C));
    func(this, nullptr, a2);
}

void FEMenu::OnR1(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x60));
    func(this, nullptr, a2);
}

void FEMenu::OnL2(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x64));
    func(this, nullptr, a2);
}

void FEMenu::OnR2(int a2)
{
    void(__fastcall * func)(FEMenu *, void *, int) = CAST(func, get_vfunc(m_vtbl, 0x68));
    func(this, nullptr, a2);
}

void FEMenu::OnAnyButtonPress(int a2, int a3)
{
    sp_log("OnAnyButtonPress:");

    if ((128 & this->field_28) != 0 && (a3 == 4 || a3 == 8 || a3 == 16 || a3 == 32) &&
        a2 == input_mgr::instance->field_58 - 1000000) {
        this->field_2A = a3;
        this->field_1C = 0.30000001;
    }
}

void FEMenu::OnButtonRelease(int a2, int a3)
{
    if ((128 & this->field_28) != 0 && (a3 == 4 || a3 == 8 || a3 == 16 || a3 == 32) &&
        a2 == input_mgr::instance->field_58 - 1000000 && a3 == this->field_2A) {
        this->field_2A = -1;
    }
}

void FEMenu::SetHigh(int a2, bool a3)
{
    sp_log("%d", a2);

    auto v4 = this->highlighted;
    if (v4 >= 0) {
        auto *v5 = this->field_4[v4];
        v5->Highlight(0, 1);
    }

    auto *v6 = this->field_4[a2];
    if (v6) {
        v6->Highlight(1, a3);
    }

    this->highlighted = a2;
}

void FEMenu::SetVis(int a2)
{
    sp_log("SetVis(): 0x%08X", m_vtbl);

    if constexpr (1) {
        auto v3 = a2;
        auto v4 = 0;
        auto v5 = this->field_26 <= 0;
        this->field_20 = a2;
        if (!v5) {
            int v11;
            int v7;

            while (1) {
                do {
                    while (v3 < this->num_entries) {
                        if ((this->field_28 & 0x40) != 0) {
                            auto &v6 = this->field_4[v3];
                            if (v6->GetDisable()) {
                                break;
                            }
                        }

                    LABEL_8:
                        if ((this->field_28 & 0x400) != 0) {
                            v7 = v4 * this->field_14;
                        LABEL_13:
                            v11 = this->field_10 + v7;
                            goto LABEL_14;
                        }

                        if ((this->field_28 & 0x800) == 0) {
                            v7 = v4 * this->field_14 - this->field_18;
                            goto LABEL_13;
                        }

                        v11 = this->field_10 + v4 * this->field_14 - 2 * this->field_18;
                    LABEL_14:

                        auto *v8 = this->field_4[v3];

                        v8->SetPos(this->field_C, v11);
                        ++v4;
                        ++v3;
                        if (v4 >= this->field_26) {
                            return;
                        }
                    }

                    ++v3;
                } while (v3 < this->num_entries);

                if ((this->field_28 & 2) == 0) {
                    goto LABEL_8;
                }

                v3 = 0;
            }
        }

    } else {
        THISCALL(0x0060AE00, this, a2);
    }
}

void FEMenu::Up()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xA4));

    func(this);
}

void FEMenu::Down()
{
    void(__fastcall * func)(void *) = CAST(func, get_vfunc(m_vtbl, 0xA8));

    func(this);
}

void FEMenu::ButtonHeldAction()
{
    if ((128 & this->field_28) != 0) {
        auto v1 = this->field_2A;
        if (v1 == 4) {
            this->Up();
        } else if (v1 == 8) {
            this->Down();
        }
    }
}

void sub_582A30()
{
    SetEvent(hEvent);
    WaitForSingleObject(hObject, 0xFFFFFFFF);
    CloseHandle(hObject);
    CloseHandle(hEvent);
    hObject = nullptr;
    hEvent = nullptr;
}

void sub_5A6D70()
{
    sp_log("sub_5A6D70");

    CDECL_CALL(0x005A6D70);
}

void FEMenu_patch()
{
    return;
    {
        REDIRECT(0x00630B61, sub_5A6D70);
        REDIRECT(0x00635CA0, sub_5A6D70);
    }

    {
        void (FEMenu::*AddEntry)(int a2, FEText *a3, bool a4) = &FEMenu::AddEntry;

        FUNC_ADDRESS(address, AddEntry);
        SET_JUMP(0x00618610, address);
    }

    {
        void (FEMenu::*AddEntry)(int a2, global_text_enum a3) = &FEMenu::AddEntry;
        FUNC_ADDRESS(address, AddEntry);

        SET_JUMP(0x00629850, address);
    }

    if constexpr (0) {
        {
            FUNC_ADDRESS(address, &FEMenu::Update);
            SET_JUMP(0x0060B180, address);
        }

        {
            FUNC_ADDRESS(address, &FEMenu::SetVis);
            SET_JUMP(0x0060AE00, address);
        }

        FUNC_ADDRESS(address, &FEMenu::SetHigh);
        set_vfunc(0x00894274, address);

        {
            FUNC_ADDRESS(address, &FEMenu::OnAnyButtonPress);
            set_vfunc(0x0089426C, address);
        }
    }
}
