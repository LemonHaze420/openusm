#include "cursor.h"

#include "common.h"
#include "func_wrapper.h"
#include "texturehandle.h"
#include "utility.h"
#include "variables.h"

#include <cassert>

#include <d3dx9tex.h>

VALIDATE_SIZE(Cursor, 0x13C);
VALIDATE_OFFSET(Cursor, field_12C, 0x12C);

Cursor *&g_cursor = var<Cursor *>(0x0096191C);

namespace {
Cursor *__fastcall destroy_cursor(Cursor *cursor, void *, unsigned char flags)
{
    cursor->~Cursor();
    if ((flags & 1) != 0)
        operator delete(cursor);
    return cursor;
}
void *native_cursor_table[]{reinterpret_cast<void *>(&destroy_cursor)};
}  // namespace

Cursor::Cursor(LPCWSTR lpWideCharStr, int a3, int a4)
{
    if constexpr (1) {
        this->m_vtbl = STANDALONE_SYSTEM ? reinterpret_cast<std::intptr_t>(native_cursor_table) : 0x0088F4F8;
        nglTexture *v5 = &this->field_7C;
        v5->FileName = {};

        auto **v6 = &this->field_14;

        lpWideCharStr = L"data\\ump.dat";

        if (auto result = D3DXCreateTextureFromFileExW(g_Direct3DDevice,
                                                       lpWideCharStr,
                                                       0,
                                                       0,
                                                       1,
                                                       0,
                                                       D3DFMT_A8R8G8B8,
                                                       D3DPOOL_MANAGED,
                                                       3,
                                                       3,
                                                       0,
                                                       nullptr,
                                                       nullptr,
                                                       &this->field_14);
            FAILED(result)) {
            assert(0);
        }

        this->field_7C.DXTexture = *v6;
        nglInitQuad(&this->field_18);
        nglSetQuadZ(&this->field_18, -9999.0);
        nglSetQuadTex(&this->field_18, v5);
        nglSetQuadUV(&this->field_18, 0.0, 0, 1.0, 1.0);
        nglSetQuadZ(&this->field_18, 1.0);
        this->m_screenWidth = a3;
        this->m_screenHeight = a4;
        this->field_14 = nullptr;
        this->field_114 = 0;
        this->field_120 = 0;
        this->field_124 = 640.0 / (double)a3;
        this->field_128 = 480.0 / (double)a4;
    } else {
        THISCALL(0x005A6670, this, lpWideCharStr, a3, a4);
    }
}

Cursor *__fastcall hookCtor(Cursor *self, void *, LPCWSTR lpWideCharStr, int a3, int a4)
{
    new (self) Cursor{lpWideCharStr, a3, a4};
    return self;
}

Cursor::~Cursor()
{
    m_vtbl = STANDALONE_SYSTEM ? reinterpret_cast<std::intptr_t>(native_cursor_table) : 0x0088F4F8;
    if (field_14 != nullptr) {
        field_14->lpVtbl->Release(field_14);
    }
}

void Cursor::Draw()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (field_114) {
            sub_581C60();
            nglSetQuadRect(&field_18,
                           static_cast<float>(field_104.x),
                           static_cast<float>(field_104.y),
                           static_cast<float>(field_104.x + 48),
                           static_cast<float>(field_104.y + 48));
            nglListAddQuad(&field_18);
        }
    } else {
        THISCALL(0x00594DF0, this);
    }
}

void Cursor::sub_5A67D0(int a1, int a2, int a3, int a4)
{
    field_12C.push_back(RECT{a1, a2, a3, a4});
}

void Cursor::sub_5A6790()
{
    _std::vector<RECT> empty;
    field_12C.swap(empty);
}

void Cursor::sub_581C60()
{
    if (!this->field_120) {
        this->field_114 = true;
    }

    GetCursorPos(&this->m_cursorLoc);

    this->field_104.x = (uint64_t)(this->m_cursorLoc.x * 640.0 / this->m_screenWidth);
    this->field_104.y = (uint64_t)(this->m_cursorLoc.y * 480.0 / this->m_screenHeight);
    ScreenToClient(g_appHwnd, &this->field_104);
}

void Cursor::sub_5B0D70()
{
    if (!this->field_120) {
        this->field_114 = false;
    }
}

void cursor_patch()
{
    REDIRECT(0x005AD24A, hookCtor);
}
