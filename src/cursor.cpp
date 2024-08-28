#include "cursor.h"

#include "common.h"
#include "func_wrapper.h"
#include "texturehandle.h"
#include "utility.h"
#include "variables.h"

#include <cassert>

#include <d3dx9tex.h>

VALIDATE_SIZE(Cursor, 0x13C);

Cursor *& g_cursor = var<Cursor *>(0x0096191C);

Cursor::Cursor(LPCWSTR lpWideCharStr, int a3, int a4) {
    if constexpr (1) {
        this->m_vtbl = 0x0088F4F8;
        nglTexture *v5 = &this->field_7C;
        v5->field_60 = {};

        this->field_130 = nullptr;
        this->field_134 = 0;
        this->field_138 = 0;
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
        this->field_124 = 640.0 / (double) a3;
        this->field_128 = 480.0 / (double) a4;
    } else {
        THISCALL(0x005A6670, this, lpWideCharStr, a3, a4);
    }
}

Cursor * __fastcall hookCtor(Cursor *self, void *, LPCWSTR lpWideCharStr, int a3, int a4) {
    new (self) Cursor{lpWideCharStr, a3, a4};
    return self;
}

Cursor::~Cursor() {
    THISCALL(0x005A6810, this);
}

void Cursor::Draw()
{
    THISCALL(0x00594DF0, this);
}

void Cursor::sub_5A67D0(int a1, int a2, int a3, int a4) {
    THISCALL(0x005A67D0, this, a1, a2, a3, a4);
}

void Cursor::sub_5A6790() {
    if (this->field_130 != nullptr) {
        operator delete(this->field_130);
    }

    this->field_130 = nullptr;
    this->field_134 = 0;
    this->field_138 = 0;
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

void Cursor::sub_5B0D70() {
    if (!this->field_120) {
        this->field_114 = false;
    }
}

void cursor_patch() {
    REDIRECT(0x005AD24A, hookCtor);
}
