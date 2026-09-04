#include "texturehandle.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"
#include "variable.h"

VALIDATE_SIZE(TextureHandle, 0x10);

TextureHandle::TextureHandle()
{
    this->field_0 = INVALID_HANDLE_VALUE;
    this->field_4 = INVALID_HANDLE_VALUE;
    this->field_8 = nullptr;
    this->m_fileSize = 0;
}

int TextureHandle::sub_7DF161(LPCWSTR lpWideCharStr, int a3)
{
    if constexpr (1) {
        HANDLE v8;

        const WCHAR *v3 = lpWideCharStr;

        CHAR *v13 = nullptr;
        if (lpWideCharStr == nullptr) {
            return 0x8876086C;
        }

        if (a3 != 0) {
            OSVERSIONINFOA VersionInformation;
            VersionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
            GetVersionExA(&VersionInformation);
            if (VersionInformation.dwPlatformId != VER_PLATFORM_WIN32_NT) {
                auto cbMultiByte = WideCharToMultiByte(CP_UTF8, 0, lpWideCharStr, -1, nullptr, 0, nullptr, nullptr);
                auto *v7 = static_cast<CHAR *>(operator new(cbMultiByte));
                v13 = v7;
                if (v7 == nullptr) {
                    return 0x8007000E;
                }

                WideCharToMultiByte(CP_UTF8, 0, lpWideCharStr, -1, v7, cbMultiByte, nullptr, nullptr);
                a3 = 0;
                v3 = (const WCHAR *)v13;
            }
        }

        if (a3) {
            v8 = CreateFileW(v3, 0x80000000, 1u, nullptr, 3u, 0x10000000u, nullptr);
        } else {
            v8 = CreateFileA((LPCSTR)v3, 0x80000000, 1u, nullptr, 3u, 0x10000000u, nullptr);
        }

        this->field_0 = v8;
        operator delete(v13);
        if (this->field_0 == INVALID_HANDLE_VALUE) {
            GetLastError();
            return 0x88760B59;
        }

        this->field_4 = CreateFileMappingA(this->field_0, nullptr, 2u, 0, 0, nullptr);
        if (field_4 == INVALID_HANDLE_VALUE) {
            GetLastError();
            return 0x88760B59;
        }

        this->m_fileSize = GetFileSize(this->field_0, nullptr);

        if (m_fileSize == -1) {
            GetLastError();
            return 0x88760B59;
        }

        if (m_fileSize) {
            this->field_8 = MapViewOfFile(this->field_4, 4, 0, 0, 0);
            if (this->field_8 == nullptr) {
                GetLastError();
                return 0x88760B59;
            }

            //sp_log("%s %d %d", (LPCSTR) v3, ((char *) this->field_8)[0], ((char *) this->field_8)[1]);

        } else {
            this->field_8 = nullptr;
        }

        return 0;
    } else {
        return THISCALL(0x007DF161, this, lpWideCharStr, a3);
    }
}

TextureHandle::~TextureHandle()
{
    if (this->field_0 != INVALID_HANDLE_VALUE) {
        auto *v2 = this->field_8;
        if (v2 != nullptr) {
            UnmapViewOfFile(v2);
            this->field_8 = nullptr;
            this->m_fileSize = 0;
        }

        if (this->field_4 != INVALID_HANDLE_VALUE) {
            CloseHandle(this->field_4);
            this->field_4 = INVALID_HANDLE_VALUE;
        }

        if (this->field_0 != INVALID_HANDLE_VALUE) {
            CloseHandle(this->field_0);
            this->field_0 = INVALID_HANDLE_VALUE;
        }
    }
}

void TextureHandle_patch()
{
    FUNC_ADDRESS(address, &TextureHandle::sub_7DF161);
    REDIRECT(0x007CA245, address);
}
