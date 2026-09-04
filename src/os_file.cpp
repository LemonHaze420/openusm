#define _WIN32_WINNT 0x0A00
#include "os_file.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "os_developer_options.h"
#include "string_hash_dictionary.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

#include <cassert>
#include <direct.h>
#include <memory>
#include <windows.h>

VALIDATE_OFFSET(os_file, field_1C, 0x1C);

#if !STANDALONE_SYSTEM
bool &os_file::system_locked = var<bool>(0x00965E68);

bool &byte_965E69 = var<bool>(0x00965E69);

auto &g_lpBuffer = var<LPVOID[10]>(0x00965E6C);

auto &byte_965E94 = var<bool[10]>(0x00965E94);

#else

static bool g_system_locked{};
bool &os_file::system_locked = g_system_locked;

static bool g_byte_965E69{};
bool &byte_965E69 = g_byte_965E69;

static LPVOID g_lpBuffer1[10]{};
LPVOID (&g_lpBuffer)[10] = g_lpBuffer1;

static bool g_byte_965E94[10]{};
bool (&byte_965E94)[10] = g_byte_965E94;

#endif

static constexpr auto max_size = 0x200000;

decltype(auto) get_file_pathname(HANDLE hFile)
{
    std::unique_ptr<char[]> buffer = std::make_unique<char[]>(MAX_PATH);

    auto result = GetFinalPathNameByHandle(hFile,  // handle to file
                                           //buffer.data(),
                                           buffer.get(),      // buffer that receives the resolved path
                                           MAX_PATH,          // max buffer size
                                           VOLUME_NAME_DOS);  // path with normal volume label

    assert(result > 0 && result <= MAX_PATH);

    return buffer;
}

void sub_58DE20()
{
    byte_965E69 = true;

    for (int i{0}; i < 10; ++i) {
        g_lpBuffer[i] = malloc(max_size);
        byte_965E94[i] = 0;
    }
}

void sub_58DE60()
{
    byte_965E69 = false;

    for (int i{0}; i < 10; ++i) {
        free(g_lpBuffer[i]);
        g_lpBuffer[i] = nullptr;
        byte_965E94[i] = false;
    }
}

void add_str(char *Dest, const char *Source, size_t a3)
{
    strncat(Dest, Source, a3 - strlen(Dest));
    Dest[a3 - 1] = '\0';
}

void copy_str(char *Dest, const char *Source, size_t Count)
{
    strncpy(Dest, Source, Count);
    Dest[Count - 1] = '\0';
}

#if !STANDALONE_SYSTEM
bool &byte_9363E8 = var<bool>(0x009363E8);
bool &byte_9363E0 = var<bool>(0x009363E0);

char (&byte_967BE0)[284] = var<char[284]>(0x00967BE0);
char (&byte_967D08)[260] = var<char[260]>(0x00967D08);
#else

static bool g_byte_9363E8 = true;
bool &byte_9363E8 = g_byte_9363E8;

static bool g_byte_9363E0 = true;
bool &byte_9363E0 = g_byte_9363E0;

static char g_byte_967BE0[284]{};
char (&byte_967BE0)[284] = g_byte_967BE0;

static char g_byte_967D08[260]{};
char (&byte_967D08)[260] = g_byte_967D08;

#endif

char *sub_598D40()
{
    if (byte_9363E8) {
        if (byte_9363E0) {
            getcwd(byte_967BE0, 260u);
            add_str(byte_967BE0, "\\", 260u);
            byte_9363E0 = false;
        }

        copy_str(byte_967D08, byte_967BE0, 260u);
        add_str(byte_967D08, "data\\", 260u);
        byte_9363E8 = false;
    }
    return byte_967D08;
}

os_file::os_file()
#ifdef _STDEX_NATIVE_CPP11_SUPPORT
    : m_path()
#endif
{
#ifndef _STDEX_NATIVE_CPP11_SUPPORT
    this->field_0 = mString();
#endif

    this->flags = 0;
    this->opened = false;
    this->field_15 = false;
    this->field_1C = 0;
    this->field_2C = 0;
    this->field_30 = 0;
    this->io = INVALID_HANDLE_VALUE;

    this->m_offset = 0xFFFFFFFF;
    this->field_24 = byte_965E69;
}

os_file::os_file(const mString &a2, int dwShareMode) : m_path()
{
    this->io = INVALID_HANDLE_VALUE;

    this->m_offset = 0xFFFFFFFF;
    this->flags = 0;
    this->opened = false;
    this->field_15 = false;
    this->field_1C = 0;
    this->field_2C = 0;
    this->field_30 = 0;
    this->field_24 = byte_965E69;
    this->open(a2, dwShareMode);
}

os_file::~os_file()
{
    if (this->opened) {
        this->close();
    }
}

void os_file::sub_58DEF0()
{
    void *v5 = g_lpBuffer[this->m_offset];

    uint32_t v7 = max_size;

    if (this->field_2C + this->field_30 + max_size > this->m_fileSize) {
        v7 = this->m_fileSize - this->field_2C - this->field_30;
    }

    auto numberOfBytesRead = 0ul;
    ReadFile(this->io, v5, v7, &numberOfBytesRead, nullptr);
    this->field_30 = numberOfBytesRead;
}

size_t os_file::sub_58DF50()
{
    LPCVOID v3 = g_lpBuffer[this->m_offset];

    auto NumberOfBytesWritten = 0ul;
    WriteFile(this->io, v3, this->field_2C, &NumberOfBytesWritten, nullptr);

    this->field_2C = 0;
    return NumberOfBytesWritten;
}

void os_file::close()
{
    assert(io != INVALID_HANDLE_VALUE);
    assert(opened);

    if (this->field_24) {
        if (!(this->flags & 1) && g_lpBuffer[this->m_offset] && this->field_2C) {
            this->sub_58DF50();
        }

        byte_965E94[this->m_offset] = 0;
    }

    CloseHandle(this->io);
    this->io = INVALID_HANDLE_VALUE;

    this->opened = 0;
    this->field_15 = true;
    this->field_1C = 0;
    this->m_fileSize = 0;
    this->m_path = "";
    this->field_30 = 0;
    this->field_2C = 0;
    this->m_offset = 0xFFFFFFFF;
}

void os_file::set_fp(uint32_t pos, os_file::filepos_t base)
{
    if (this->field_24 && this->flags & 6 && this->field_2C) {
        this->sub_58DF50();
    }

    auto method = 0ul;
    if (base) {
        if (base == FP_CURRENT) {
            method = 1;
            this->field_1C += pos;
        } else if (base == FP_END) {
            method = 2;
            this->field_1C = pos + this->m_fileSize;
        }
    } else {
        method = 0;
        this->field_1C = pos;
    }

    SetFilePointer(this->io, static_cast<int>(pos), nullptr, method);
    if (this->field_24) {
        if (this->flags & 1) {
            this->field_2C = this->field_1C;
            this->field_30 = 0;
            this->sub_58DEF0();
        }
    }
}

int os_file::read(LPVOID data, int bytes)
{
    TRACE("os_file::read");

    if constexpr (1) {
        int v3 = bytes;
        if (bytes == 0) {
            return 0;
        }

        if (this->field_1C + bytes > this->m_fileSize) {
            v3 = this->m_fileSize - this->field_1C;
        }

        char *buffer = static_cast<char *>(data);
        bytes = 0;

        if (v3 > max_size) {
            uint32_t count = (static_cast<uint32_t>(v3 - (max_size + 1)) >> 21) + 1;
            assert(count == static_cast<uint32_t>(v3 / max_size));

            v3 += -max_size * count;

            if constexpr (0) {
                do {
                    buffer += max_size;
                    --count;
                    bytes += this->read(buffer, max_size);
                } while (count);
            } else {
                for (auto i = count; i != 0; --i) {
                    auto ret = this->read(buffer, max_size);
                    buffer += max_size;
                    bytes += ret;
                }
            }
        }

        assert(!this->field_24);

        if (this->field_24) {
            if (this->m_fileSize > 0 && v3 > 0) {
                data = g_lpBuffer[this->m_offset];
                if (v3 + this->field_1C > this->field_2C + this->field_30) {
                    auto v13 = this->field_2C - this->field_1C;
                    auto v14 = (this->field_30 + v13 == 0);
                    auto v15 = this->field_30 + v13;

                    if (!v14) {
                        buffer += v15;
                        bytes += this->read(buffer, v15);
                        v3 -= v15;
                    }

                    this->sub_58DEF0();
                    this->field_2C += this->field_30;
                }

                std::memcpy(buffer, static_cast<char *>(data) + this->field_1C - this->field_2C, v3);

                bytes += v3;
                this->field_1C += v3;
            }
        } else {
            auto numberOfBytesRead = 0ul;
            ReadFile(this->io, buffer, v3, &numberOfBytesRead, nullptr);

            bytes += numberOfBytesRead;
            this->field_1C += numberOfBytesRead;
        }

        this->field_15 = (this->m_fileSize <= this->field_1C);
        return bytes;
    } else {
        auto result = THISCALL(0x00598AD0, this, data, bytes);

#if 0
    {
        auto *buf = static_cast<char *>(data);
        sp_log("os_file::read(): bytes = %d, data = %c %c %c", bytes, buf[0], buf[1], buf[2]);
    }
#endif

        return result;
    }
}

int os_file::get_size()
{
    int result = -1;
    if (this->opened) {
        result = this->m_fileSize;
    }
    return result;
}

static cdecl_call &dword_965EA0 = var<cdecl_call>(0x00965EA0);

void os_file::open(const mString &path, int shareMode)
{
    TRACE("os_file::open", path.c_str());

    if constexpr (1) {
        if (os_file::system_locked && !os_developer_options::instance->get_flag(mString{"MOVE_EDITOR"})) {
            mString a1 = path + ": os_file system is locked; no file access allowed";
        }

        this->m_path = path;

        {
            auto *v4 = this->m_path.c_str();
            if (v4[0] != '\\' && v4[1] != ':') {
                char *v5 = sub_598D40();

                mString v6 = v5 + this->m_path;
                this->m_path = v6;
            }
        }

        uint32_t dwDesiredAccess = 0xFFFFFFFF;
        uint32_t dwCreationDisposition = 0xFFFFFFFF;
        uint32_t dwShareMode = 0xFFFFFFFF;
        this->flags = shareMode;

        switch (shareMode) {
        case 1: {
            dwCreationDisposition = OPEN_EXISTING;
            dwDesiredAccess = GENERIC_READ;
            dwShareMode = FILE_SHARE_READ;
        } break;
        case 2: {
            dwCreationDisposition = CREATE_ALWAYS;
            dwDesiredAccess = GENERIC_WRITE;
            dwShareMode = 0;
        } break;
        case 3:
        case 4: {
            dwCreationDisposition = OPEN_ALWAYS;
            dwDesiredAccess = GENERIC_WRITE;
            dwShareMode = 0;
        } break;
        default: {
            dwDesiredAccess = (GENERIC_READ | GENERIC_WRITE);
            dwCreationDisposition = OPEN_ALWAYS;
            dwShareMode = 0;
        } break;
        };

        this->field_30 = 0;

        this->io = CreateFileA(this->m_path.c_str(),
                               dwDesiredAccess,
                               dwShareMode,
                               nullptr,
                               dwCreationDisposition,
                               FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_NORMAL,
                               nullptr);

        if (this->io == INVALID_HANDLE_VALUE) {
            this->opened = false;
        } else {
            if (this->flags == 1) {
                this->m_fileSize = GetFileSize(this->io, nullptr);

                this->field_30 = 0;
            } else {
                this->m_fileSize = 0;
                this->field_30 = max_size;
            }

            bool v12 = (this->field_24 == 0);
            this->field_2C = 0;
            if (!v12) {
                uint32_t v13 = 0;

                [&]() {
                    while (byte_965E94[v13]) {
                        if (++v13 >= 10) {
                            return;
                        }
                    }

                    this->m_offset = v13;
                }();

                byte_965E94[v13] = 1;
            }

            this->opened = true;
            this->field_1C = 0;
            if (this->flags == 4) {
                if (this->field_24 && this->field_2C) {
                    this->sub_58DF50();
                }

                this->field_1C = this->m_fileSize;
                SetFilePointer(this->io, 0, nullptr, FILE_END);
                if (this->field_24 && this->flags & 1) {
                    this->field_2C = this->field_1C;
                    this->field_30 = 0;
                    this->sub_58DEF0();
                }
            }

            this->field_15 = (this->field_1C >= this->m_fileSize);
        }

        if (dword_965EA0 != nullptr && this->flags == 2) {
            if (this->opened) {
                dword_965EA0(this);
            }
        }
    } else {
        void(__fastcall * func)(void *, void *edx, const mString *path, int shareMode) = CAST(func, 0x0059B740);
        func(this, nullptr, &path, shareMode);
    }
}

int os_file::write(const void *lpBuffer, int nNumberOfBytesToWrite)
{
    if constexpr (1) {
        auto v3 = nNumberOfBytesToWrite;

        if (!nNumberOfBytesToWrite) {
            return 0;
        }

        auto *buffer = static_cast<const char *>(lpBuffer);
        nNumberOfBytesToWrite = 0;
        if (v3 > max_size) {
            auto v7 = ((unsigned int)(v3 - (max_size + 1)) >> 21) + 1;
            v3 -= max_size * v7;
            do {
                auto v8 = this->write(buffer, max_size);
                buffer += max_size;
                --v7;
                nNumberOfBytesToWrite += v8;
            } while (v7);
        }

        if (this->field_24) {
            auto v10 = v3 + this->field_2C;
            lpBuffer = g_lpBuffer[this->m_offset];
            if (v10 > this->field_30) {
                this->sub_58DF50();
            }

            std::memcpy(bit_cast<char *>(lpBuffer) + this->field_2C, buffer, v3);
            auto v11 = this->field_1C;
            auto v12 = v3 + nNumberOfBytesToWrite;
            this->field_2C += v3;
            nNumberOfBytesToWrite = v12;
            this->field_1C = v3 + v11;
        } else {
            auto v13 = this->io;
            auto numberOfBytesWritten = 0ul;
            WriteFile(v13, buffer, v3, &numberOfBytesWritten, nullptr);
            auto v14 = this->field_1C;
            nNumberOfBytesToWrite += numberOfBytesWritten;
            this->field_1C = (int)lpBuffer + v14;
        }

        auto v15 = this->field_1C;
        if (v15 >= this->m_fileSize) {
            this->m_fileSize = v15;
        }

        this->field_15 = (this->m_fileSize <= v15);

        return nNumberOfBytesToWrite;
    } else {
        return (int)THISCALL(0x00598C30, lpBuffer, nNumberOfBytesToWrite);
    }
}

void os_file_patch()
{
    {
        FUNC_ADDRESS(address, &os_file::write);
        REDIRECT(0x0052A72B, address);
        REDIRECT(0x0052A74C, address);
        REDIRECT(0x0052A79A, address);
    }

    {
        FUNC_ADDRESS(address, &os_file::read);
        REDIRECT(0x00550E8E, address);
        REDIRECT(0x00598B28, address);
    }

    {
        FUNC_ADDRESS(address, &os_file::open);
        SET_JUMP(0x0059B740, address);
    }
}
