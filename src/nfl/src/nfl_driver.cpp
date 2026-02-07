#include "nfl_driver.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "trace.h"
#include "nfl_system.h"
#include "return_address.h"
#include "utility.h"
#include "variable.h"
#include "variables.h"

#include <cstring>
#include <iomanip>
#include <sstream>

int nflBufferAlign(
    nflDriver *driver, nflCommand *ioCommand, uint32_t a3, uint32_t *a4, uint32_t *a5) {
    assert(ioCommand != nullptr && driver != nullptr && driver->init != nullptr &&
           driver->buffer != nullptr);

    if constexpr (1) {
        auto *buffer = driver->buffer;
        uint32_t v6 = ioCommand->field_4;
        auto *v7 = ioCommand->field_10;
        int v8 = ioCommand->field_14;
        *a4 = 0;
        *a5 = 0;
        if (!v8) {
            return 0;
        }

        int v10 = (int) &v7[buffer->field_0 - 1];
        int v11 = buffer->field_4;
        int v12 = ~(buffer->field_0 - 1) & v10;
        uint32_t v13 = v6 & ~(buffer->field_8 - 1);
        int v14 = ~(v11 - 1);

        int v15 = v14 & (v8 + v6 + v11 - v13 - 1);
        uint32_t v16 = v14 & (v11 + a3 - 1);
        if (v15 > buffer->field_C) {
            v15 = buffer->field_C;
        }

        if (!ioCommand->field_18) {
            if (v6 >= a3) {
                return 0;
            }

            if (v15 + v13 > v16) {
                v15 = v16 - v13;
            }

            v15 = ~(buffer->field_4 - 1) & (buffer->field_4 + v15 - 1);
        }

        *a4 = v13;
        *a5 = v15;
        int v17 = driver->init->field_0;
        int result;

        switch (v17) {
        case 1:
        case 2:
        case 4: {
            int v18 = 0;
            if (v7 != (char *) v12) {
                v18 = 1;
                sp_log("nfdBufferAlign: Unaligned access to 0x%08X, should be aligned to 0x%08X\n",
                       v7,
                       buffer[0])
            }

            if (((buffer->field_4 - 1) & v8) != 0) {
                v18 |= 2u;
                sp_log(
                    "nfdBufferAlign: Unaligned transfer of 0x%08X bytes, should be aligned to "
                    "0x%08X\n",
                    v8,
                    buffer[1]);
            }

            if (v6 != v13) {
                v18 |= 4u;
                sp_log(
                    "nfdBufferAlign: Unaligned seek to offset 0x%08X, should be aligned to "
                    "0x%08X\n",
                    v6,
                    buffer[2])
            }

            if (v17 == 1) {
                result = -(v18 != 0);
            } else {
                result = v18 != 0;
            }
        } break;
        case 3:
            result = 1;
            break;
        default:
            result = -1;
            break;
        }
        return result;

    } else {
        return CDECL_CALL(0x0079FC50, driver, ioCommand, a3, a4, a5);
    }
}

void nfdIoComplete(nflDriver *driver, uint32_t dwNumberOfBytesTransfered, int a3) {
    if constexpr (1) {
        if (driver->field_40 == NFD_IO_STATE_WORKING) {
            if (a3) {
                driver->field_40 = NFD_IO_STATE_ERROR;
            } else if (driver->field_44) {
                int v3 = driver->field_24.field_4 - driver->field_18;
                int v4 = dwNumberOfBytesTransfered - v3;

                if (v4 > driver->field_24.field_14) {
                    v4 = driver->field_24.field_14;
                }

                memcpy((void *) driver->field_24.field_10, &driver->buffer->field_10[v3], v4);
                driver->field_1C = v4;
                driver->field_40 = NFD_IO_STATE_WORKDONE;
            } else {
                driver->field_1C = dwNumberOfBytesTransfered;
                driver->field_40 = NFD_IO_STATE_WORKDONE;
            }
        } else if (driver->field_40 == NFD_IO_STATE_CANCELING) {
            driver->field_40 = NFD_IO_STATE_CANCELED;
        }
    } else {
        CDECL_CALL(0x0079FE80, driver, dwNumberOfBytesTransfered, a3);
    }
}

#if !STANDALONE_SYSTEM

static nflDriver *& dword_984550 = var<nflDriver *>(0x00984550);

static auto & hThread = var<HANDLE>(0x00987504);

static auto & dwMilliseconds = var<DWORD>(0x00955194);

static auto & g_WriteFile = var<BOOL (__stdcall *)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)>(0x009551A4);

static auto & g_WriteFileEx = var<BOOL(__stdcall *)(HANDLE, LPCVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE)>(0x009551A8);

static auto & g_GetOverlappedResult = var<BOOL (__stdcall *)(HANDLE, LPOVERLAPPED, LPDWORD, BOOL)>(0x00987514);

static auto & g_CancelIo = var<BOOL (__stdcall *)(HANDLE)>(0x0098750C);

static auto & g_CloseHandle = var<BOOL (__stdcall *)(HANDLE hObject)>(0x00987510);

static auto & g_CreateFileA = var<HANDLE (__stdcall *)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)>(0x00955198);

static auto & g_ReadFile = var<BOOL (__stdcall *)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)>(0x0095519C);

static auto & g_ReadFileEx = var<BOOL(__stdcall *)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE)>(0x009551A0);

static auto & dword_955190 = var<int>(0x00955190);

static auto & dword_9874FC = var<void *>(0x009874FC);

static auto & dword_9874F8 = var<int>(0x009874F8);

static auto & dword_987500 = var<HANDLE>(0x00987500);

static auto & dword_987508 = var<int>(0x00987508);

static auto & byte_95518C = var<bool>(0x0095518C);

#else

static nflDriver *& dword_984550 = []() -> auto & {
    static nflDriver * g_dword_984550 {};
    return g_dword_984550;
}();

static auto & hThread = []() -> auto & {
    static HANDLE g_hThread {};
    return g_hThread;
}();

static auto & dwMilliseconds = []() -> auto & {
    static DWORD g_dwMilliseconds {1};
    return g_dwMilliseconds;
}();

static auto & g_WriteFile = []() -> auto & {
	static BOOL (__stdcall *g_WriteFile1)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) {};
	return g_WriteFile1;
}();

static auto & g_WriteFileEx = []() -> auto & {
    static BOOL (__stdcall *g_WriteFileEx1)(HANDLE, LPCVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
    return g_WriteFileEx1;
}();

static auto & g_GetOverlappedResult = []() -> auto & {
	static BOOL (__stdcall * g_GetOverlappedResult1)(HANDLE, LPOVERLAPPED, LPDWORD, BOOL) {};
	return g_GetOverlappedResult1;
}();

static auto & g_CancelIo = []() -> auto & {
    static BOOL (__stdcall *g_CancelIo1)(HANDLE) {};
    return g_CancelIo1;
}();

static auto & g_CloseHandle = []() -> auto & {
    static BOOL (__stdcall *g_CloseHandle1)(HANDLE hObject) {};
    return g_CloseHandle1;
}();

static auto & g_CreateFileA = []() -> auto & {
	static HANDLE (__stdcall *g_CreateFileA1)(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {};
	return g_CreateFileA1;
}();

static auto & g_ReadFile = []() -> auto & {
	static BOOL (__stdcall *g_ReadFile1)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) {};
	return g_ReadFile1;
}();

static auto & g_ReadFileEx = []() -> auto & {
    static BOOL (__stdcall *g_ReadFileEx1)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
    return g_ReadFileEx1;
}();

static auto & dword_955190 = []() -> auto & {
    static int g_dword_955190 {8192};
    return g_dword_955190;
}();

static auto & dword_9874FC = []() -> auto & {
    static void * g_dword_9874FC {};
    return g_dword_9874FC;
}();

static auto & dword_9874F8 = []() -> auto & {
    static int g_dword_9874F8 {};
    return g_dword_9874F8;
}();

static auto & dword_987500 = []() -> auto & {
    static HANDLE g_dword_987500 {};
    return g_dword_987500;
}();

static auto & dword_987508 = []() -> auto & {
    static int g_dword_987508 {};
    return g_dword_987508;
}();

static auto & byte_95518C = []() -> auto & {
    static bool g_byte_95518C {true};
    return g_byte_95518C;
}();

#endif

void __stdcall nfd_win32_IoCompletionRoutine(DWORD dwErrorCode,
                                 DWORD numberOfBytesTransfered,
                                 [[maybe_unused]] LPOVERLAPPED lpOverlapped) {
    //sp_log("numberOfBytesTransfered = %u", numberOfBytesTransfered);

    nfdIoComplete(dword_984550, numberOfBytesTransfered, dwErrorCode);
}

int sub_7A00C0() {
    SleepEx(0, 1);
    return 0;
}

BOOL sub_7A00A0(HANDLE *a1) {
    return g_CancelIo(*a1) == 0;
}

BYTE * sub_81D540(int *a1, int a2, char a3, int a4)
{
    if constexpr (1) {

        BYTE *result {};

        auto v5 = *a1;
        if ( *a1 )
        {
            while ( *(int *)v5 != a2 )
            {
                v5 = *(DWORD *)(v5 + 12);
                if ( !v5 ) {
                    goto LABEL_6;
                }
            }

            *(BYTE *)(v5 + 4) = a3;
            *(DWORD *)(v5 + 8) = a4;
        }
LABEL_6:

        auto v6 = *a1;
        if ( *a1 )
        {
            int v8;
            do
            {
                v8 = v6;
                v6 = *(DWORD *)(v6 + 12);
            }
            while ( v6 );

            BYTE *result = static_cast<BYTE *>(operator new(0x10u));
            if ( result != nullptr )
            {
                *(DWORD *)result = a2;
                result[4] = a3;
                *((DWORD *)result + 2) = a4;
                *(DWORD *)(v8 + 12) = int(result);
            }
            else
            {
                result = nullptr;
                *(DWORD *)(v8 + 12) = 0;
            }
        }
        else
        {
            BYTE *result = static_cast<BYTE *>(operator new(0x10u));
            if ( result != nullptr )
            {
                *(DWORD *)result = a2;
                result[4] = a3;
                *((DWORD *)result + 2) = a4;
                *a1 = (int)result;
            }
            else
            {
                result = nullptr;
                *a1 = 0;
            }
        }

        return result;
    } else {

        BYTE * (__fastcall *func)(int *a1, void *, int a2, char a3, int a4) = CAST(func, 0x0081D540);
        return func(a1, nullptr, a2, a3,a4);
    }
}

HANDLE __stdcall sub_81D790(
        LPCSTR lpFileName,
        DWORD dwDesiredAccess,
        DWORD dwShareMode,
        LPSECURITY_ATTRIBUTES lpSecurityAttributes,
        DWORD dwCreationDisposition,
        DWORD dwFlagsAndAttributes,
        HANDLE a7)
{
    if constexpr (1) {
        auto v7 = dwFlagsAndAttributes;
        char v10 = 0;
        if ( (dwFlagsAndAttributes & 0x40000000) != 0 )
        {
            v7 = dwFlagsAndAttributes & 0x9FFFFFFF;
            v10 = 1;
        }

        auto result = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, v7, nullptr);
        auto v9 = (int)result;
        if ( result != INVALID_HANDLE_VALUE )
        {
            SetLastError(ERROR_IO_PENDING);
            WaitForSingleObject(dword_987500, 0xFFFFFFFF);
            sub_81D540((int *)&dword_9874FC, v9, v10, 0);
            ReleaseMutex(dword_987500);
            return (HANDLE)v9;
        }

        return result;
    } else {
        HANDLE (__stdcall *func)(
                LPCSTR lpFileName,
                DWORD dwDesiredAccess,
                DWORD dwShareMode,
                LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                DWORD dwCreationDisposition,
                DWORD dwFlagsAndAttributes,
                HANDLE a7) = CAST(func, 0x0081D790);

        return func(lpFileName,
                dwDesiredAccess,
                dwShareMode,
                lpSecurityAttributes,
                dwCreationDisposition,
                dwFlagsAndAttributes,
                a7);
    }
}

BOOL __stdcall sub_81D820(
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPDWORD lpNumberOfBytesRead,
        LPOVERLAPPED a5)
{
    if constexpr (1) {
        WaitForSingleObject(dword_987500, 0xFFFFFFFF);
        DWORD *v5 = static_cast<DWORD *>(dword_9874FC);
        HANDLE v6;
        DWORD *v7;
        if ( dword_9874FC )
        {
            while ( 1 )
            {
                v6 = hFile;
                if ( (HANDLE)*v5 == hFile ) {
                    break;
                }

                v5 = (DWORD *)v5[3];
                if ( !v5 ) {
                    goto LABEL_7;
                }
            }

            v7 = v5;
        }
        else
        {
            v6 = hFile;
LABEL_7:
            v7 = nullptr;
        }

        ReleaseMutex(dword_987500);
        if ( *((BYTE *)v7 + 4) )
        {
            if ( lpNumberOfBytesRead ) {
                *lpNumberOfBytesRead = 0;
            }

            a5->Internal = 259;
            a5->InternalHigh = dword_987508++;

            DWORD *v9 = static_cast<DWORD *>(operator new(0x20u));
            v9[0] = int(hFile);
            v9[1] = int(lpBuffer);
            v9[2] = int(lpBuffer);
            v9[4] = 0;
            v9[3] = int(a5);
            v9[6] = nNumberOfBytesToRead;
            v9[7] = 0;
            v9[5] = 0;
            WaitForSingleObject(dword_987500, 0xFFFFFFFF);
            sub_81D540((int *)&dword_9874FC, a5->InternalHigh, 1, (int)v9);
            ReleaseMutex(dword_987500);
            SetLastError(ERROR_IO_PENDING);
            return 0;
        }
        else
        {
            if ( a5 ) {
                SetFilePointer(v6, a5->Offset, nullptr, 0);
            }

            return ReadFile(v6, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, nullptr);
        }
    } else {

        BOOL (__stdcall *func)(
                HANDLE hFile,
                LPVOID lpBuffer,
                DWORD nNumberOfBytesToRead,
                LPDWORD lpNumberOfBytesRead,
                LPOVERLAPPED a5) = CAST(func, 0x0081D820);

        return func(hFile,
                lpBuffer,
                nNumberOfBytesToRead,
                lpNumberOfBytesRead,
                a5);
    }
}

BOOL __stdcall sub_81D950(
        HANDLE hFile,
        LPVOID lpBuffer,
        DWORD nNumberOfBytesToRead,
        LPOVERLAPPED a4,
        LPOVERLAPPED_COMPLETION_ROUTINE a5)
{
    if constexpr (1) {
        WaitForSingleObject(dword_987500, 0xFFFFFFFF);
        DWORD *v5 = static_cast<DWORD *>(dword_9874FC);
        DWORD *v6;
        if ( dword_9874FC )
        {
            while ( (HANDLE)*v5 != hFile )
            {
                v5 = (DWORD *)v5[3];
                if ( !v5 ) {
                    goto LABEL_4;
                }
            }

            v6 = v5;
        }
        else
        {
LABEL_4:
            v6 = nullptr;
        }

        DWORD NumberOfBytesRead {};
        ReleaseMutex(dword_987500);
        if ( *((BYTE *)v6 + 4) )
        {
            a4->Internal = 259;
            a4->InternalHigh = dword_987508;
            NumberOfBytesRead = 0;
            ++dword_987508;
            DWORD *v8 = static_cast<DWORD *>(malloc(0x20u));
            v8[0] = int(hFile);
            v8[1] = int(lpBuffer);
            v8[2] = int(lpBuffer);
            v8[3] = int(a4);
            v8[4] = 0;
            v8[5] = 0;
            v8[6] = nNumberOfBytesToRead;
            v8[7] = int(a5);
            WaitForSingleObject(dword_987500, 0xFFFFFFFF);
            sub_81D540((int *)&dword_9874FC, a4->InternalHigh, 1, (int)v8);
            ReleaseMutex(dword_987500);
            SetLastError(ERROR_IO_PENDING);
            return 0;
        }
        else
        {
            if ( a4 ) {
                SetFilePointer(hFile, a4->Offset, nullptr, 0);
            }

            return ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &NumberOfBytesRead, nullptr);
        }

    } else {

        BOOL (__stdcall *func)(
                HANDLE hFile,
                LPVOID lpBuffer,
                DWORD nNumberOfBytesToRead,
                LPOVERLAPPED a4,
                LPOVERLAPPED_COMPLETION_ROUTINE a5) = CAST(func, 0x0081D950);
        return func(hFile,
                lpBuffer,
                nNumberOfBytesToRead,
                a4,
                a5);
    }
}

BOOL __stdcall sub_81DA70(
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPDWORD lpNumberOfBytesWritten,
        LPOVERLAPPED a5)
{
    if constexpr (1) {
        WaitForSingleObject(dword_987500, 0xFFFFFFFF);
        DWORD *v5 = static_cast<DWORD *>(dword_9874FC);
        DWORD *v6;
        if ( dword_9874FC )
        {
            while ( (HANDLE)*v5 != hFile )
            {
                v5 = (DWORD *)v5[3];
                if ( !v5 ) {
                    goto LABEL_4;
                }
            }
            v6 = v5;
        }
        else
        {
LABEL_4:
            v6 = nullptr;
        }

        ReleaseMutex(dword_987500);
        if ( *((BYTE *)v6 + 4) )
        {
            if ( lpNumberOfBytesWritten ) {
                *lpNumberOfBytesWritten = 0;
            }

            a5->Internal = 259;
            a5->InternalHigh = dword_987508++;
            DWORD *v8 = static_cast<DWORD *>(malloc(0x20u));
            v8[0] = int(hFile);
            v8[1] = int(lpBuffer);
            v8[2] = int(lpBuffer);
            v8[3] = int(a5);
            v8[4] = 1;
            v8[5] = 0;
            v8[6] = nNumberOfBytesToWrite;
            v8[7] = 0;

            WaitForSingleObject(dword_987500, 0xFFFFFFFF);
            sub_81D540((int *)&dword_9874FC, a5->InternalHigh, 1, (int)v8);
            ReleaseMutex(dword_987500);
            SetLastError(ERROR_IO_PENDING);
            return 0;
        }
        else
        {
            if ( a5 ) {
                SetFilePointer(hFile, a5->Offset, nullptr, 0);
            }

            return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, nullptr);
        }
    } else {

        BOOL (__stdcall *func)(
                HANDLE hFile,
                LPCVOID lpBuffer,
                DWORD nNumberOfBytesToWrite,
                LPDWORD lpNumberOfBytesWritten,
                LPOVERLAPPED a5) = CAST(func, 0x0081DA70);

        return func(hFile,
                lpBuffer,
                nNumberOfBytesToWrite,
                lpNumberOfBytesWritten,
                a5);
    }
}

BOOL __stdcall sub_81DB90(
        HANDLE hFile,
        LPCVOID lpBuffer,
        DWORD nNumberOfBytesToWrite,
        LPOVERLAPPED a4,
        LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
    if constexpr (1) {
        WaitForSingleObject(dword_987500, 0xFFFFFFFF);
        DWORD *v5 = static_cast<DWORD *>(dword_9874FC);
        DWORD *v6;
        if ( dword_9874FC )
        {
            while ( (HANDLE)*v5 != hFile )
            {
                v5 = (DWORD *)v5[3];
                if ( !v5 ) {
                    goto LABEL_4;
                }
            }

            v6 = v5;
        }
        else
        {
LABEL_4:
            v6 = nullptr;
        }

        ReleaseMutex(dword_987500);
        if ( *((BYTE *)v6 + 4) )
        {
            a4->Internal = 259;
            a4->InternalHigh = dword_987508++;
            DWORD *v8 = static_cast<DWORD *>(malloc(0x20u));
            v8[0] = int(hFile);
            v8[1] = int(lpBuffer);
            v8[2] = int(lpBuffer);
            v8[3] = int(a4);
            v8[4] = 1;
            v8[5] = 0;
            v8[6] = nNumberOfBytesToWrite;
            v8[7] = int(lpCompletionRoutine);

            WaitForSingleObject(dword_987500, 0xFFFFFFFF);
            sub_81D540((int *)&dword_9874FC, a4->InternalHigh, 1, (int)v8);
            ReleaseMutex(dword_987500);
            SetLastError(ERROR_IO_PENDING);
            return 0;
        }
        else
        {
            if ( a4 ) {
                SetFilePointer(hFile, a4->Offset, nullptr, 0);
            }

            return WriteFileEx(hFile, lpBuffer, nNumberOfBytesToWrite, nullptr, lpCompletionRoutine);
        }

    } else {

        BOOL (__stdcall *func)(
                HANDLE hFile,
                LPCVOID lpBuffer,
                DWORD nNumberOfBytesToWrite,
                LPOVERLAPPED a4,
                LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine) = CAST(func, 0x0081DB90);

        return func(hFile,
                lpBuffer,
                nNumberOfBytesToWrite,
                a4,
                lpCompletionRoutine);
    }
}

void sub_81D5E0(DWORD *a1, int a2)
{
    if constexpr (1) {
        auto *v2 = (int *)*a1;
        if ( *a1 )
        {
            if ( *v2 == a2 )
            {
                auto v5 = (DWORD *)*a1;
                *a1 = v2[3];
                operator delete(v5);
            }
            else
            {
                auto v3 = (DWORD *)*a1;
                auto *v4 = (DWORD *)v2[3];
                if ( v4 != nullptr )
                {
                    while ( *(int *)v4 != a2 )
                    {
                        v3 = v4;
                        v4 = (DWORD *)v4[3];
                        if ( !v4 ) {
                            return;
                        }
                    }

                    v3[3] = v4[3];
                    operator delete(v4);
                }
            }
        }
    } else {
        void (__fastcall *func)(DWORD *a1, void *edx, int a2) = CAST(func, 0x0081D5E0);
        func(a1, nullptr, a2);
    }
}

BOOL __stdcall sub_81DCF0(HANDLE a1, LPOVERLAPPED a2, LPDWORD a3, BOOL a4)
{
    if constexpr (1) {
        WaitForSingleObject(dword_987500, 0xFFFFFFFF);
        auto *v4 = static_cast<DWORD *>(dword_9874FC);
        if ( dword_9874FC )
        {
            while ( *v4 != *(DWORD *)(a2 + 4) )
            {
                v4 = (DWORD *)v4[3];
                if ( !v4 ) {
                    goto LABEL_4;
                }
            }
        }
        else
        {
LABEL_4:
            v4 = nullptr;
        }

        ReleaseMutex(dword_987500);

        if ( v4 != nullptr )
        {
            if ( a4 )
            {
                while ( 1 )
                {
                    WaitForSingleObject(dword_987500, 0xFFFFFFFF);
                    auto *v6 = static_cast<DWORD *>(dword_9874FC);
                    DWORD *v4;
                    if ( dword_9874FC != nullptr )
                    {
                        while ( *v6 != *(DWORD *)(a2 + 4) )
                        {
                            v6 = (DWORD *)v6[3];
                            if ( !v6 ) {
                                goto LABEL_11;
                            }
                        }

                        v4 = v6;
                    }
                    else
                    {
LABEL_11:
                        v4 = nullptr;
                    }

                    ReleaseMutex(dword_987500);

                    if ( !v4 ) {
                        break;
                    }

                    if ( *(int *)(v4[2] + 16) >= 2 ) {
                        goto LABEL_17;
                    }

                    Sleep(5u);
                }

                SetLastError(ERROR_INVALID_HANDLE);
                return 0;
            }
            else
            {
LABEL_17:
                int v7 = v4[2];
                if ( a3 ) {
                    *a3 = *(DWORD *)(v7 + 20);
                }

                int v8 = *(DWORD *)(v7 + 16);
                if ( v8 == 4 )
                {
                    SetLastError(ERROR_HANDLE_EOF);
                    return 0;
                }

                else if ( v8 == 3 )
                {
                    WaitForSingleObject(dword_987500, 0xFFFFFFFF);
                    auto *v9 = static_cast<DWORD *>(dword_9874FC);
                    if ( dword_9874FC )
                    {
                        do
                        {
                            auto *v10 = v9;
                            auto *v11 = (DWORD *)v9[2];
                            v9 = (DWORD *)v9[3];
                            if ( *(DWORD *)(v11[3] + 4) == *(DWORD *)(a2 + 4) )
                            {
                                if ( v10 == (DWORD *)dword_9874F8 ) {
                                    dword_9874F8 = (int)v9;
                                }

                                sub_81D5E0((DWORD *)&dword_9874FC, *v10);
                                free(v11);
                            }
                        }
                        while ( v9 );
                    }

                    ReleaseMutex(dword_987500);
                    return true;
                }
                else
                {
                    SetLastError(ERROR_IO_INCOMPLETE);
                    return false;
                }
            }
        }
        else
        {
            SetLastError(ERROR_INVALID_HANDLE);
            return false;
        }
    } else {

        BOOL (__stdcall *func)(HANDLE a1, LPOVERLAPPED a2, LPDWORD a3, BOOL a4) = CAST(func, 0x0081DCF0);
        return func(a1, a2, a3, a4);
    }
}

BOOL __stdcall sub_81DCA0(HANDLE a1)
{
	if constexpr (1) {
		WaitForSingleObject(dword_987500, 0xFFFFFFFF);
		auto *v1 = static_cast<DWORD *>(dword_9874FC);
		if ( dword_9874FC )
		{
			do
			{
				auto v2 = v1[2];
				if ( *(HANDLE *)v2 == a1 ) {
					*(DWORD *)(v2 + 16) = 2;
				}

				v1 = (DWORD *)v1[3];
			}
			while ( v1 );
		}

		ReleaseMutex(dword_987500);
		return true;
	} else {

		BOOL (__stdcall *func)(HANDLE a1) = CAST(func, 0x0081DCA0);
		return func(a1);
	}
}

BOOL __stdcall sub_81DE70(HANDLE hObject)
{
	if constexpr (1) {
		WaitForSingleObject(dword_987500, 0xFFFFFFFF);
		auto v1 = dword_9874FC;
        HANDLE *v3;
        int *v2;
		if ( dword_9874FC )
		{
			while ( 1 )
			{
				v2 = (int *)v1;
				v3 = (HANDLE *)*((DWORD *)v1 + 2);
				v1 = (void *)*((DWORD *)v1 + 3);
				if ( *v3 == hObject ) {
					break;
				}
LABEL_15:
				if ( !v1 ) {
					goto LABEL_16;
				}
			}

			if ( v2 == (int *)dword_9874F8 ) {
				dword_9874F8 = (int)v1;
			}

			auto v4 = *v2;
			auto *v5 = static_cast<DWORD *>(dword_9874FC);
			if ( dword_9874FC )
			{
				if ( *(int *)dword_9874FC == v4 )
				{
					dword_9874FC = (void *)*((DWORD *)dword_9874FC + 3);
LABEL_13:
					operator delete(v5);
					goto LABEL_14;
				}

				auto *v6 = static_cast<DWORD *>(dword_9874FC);
				v5 = (DWORD *)*((DWORD *)dword_9874FC + 3);
				if ( v5 != nullptr )
				{
					while ( *(int *)v5 != v4 )
					{
						v6 = v5;
						v5 = (DWORD *)v5[3];
						if ( !v5 ) {
							goto LABEL_14;
						}
					}

					v6[3] = v5[3];
					goto LABEL_13;
				}
			}
LABEL_14:
			free(v3);
			goto LABEL_15;
		}
LABEL_16:

		ReleaseMutex(dword_987500);
		return CloseHandle(hObject);
	} else {
		BOOL (__stdcall *func)(HANDLE) = CAST(func, 0x0081DE70);
		return func(hObject);
	}
}

BOOL nfd_win32_IoExecute(
    nflDriver *a1, HANDLE *a2, nflRequestType requestType, uint32_t a4, LPVOID lpBuffer, uint32_t nNumberOfBytes)
{
    TRACE("nfd_wind32_IoExecute");

    assert(requestType == NFL_REQUEST_TYPE_READ
            || requestType == NFL_REQUEST_TYPE_WRITE);

    if constexpr (1)
    {
#if !STANDALONE_SYSTEM
        auto & Overlapped = var<_OVERLAPPED>(0x0098453C);

#else

        _OVERLAPPED & Overlapped = []() -> auto & {
            static _OVERLAPPED g_Overlapped {};
            return g_Overlapped;
        }();

#endif


        dword_984550 = a1;
        Overlapped.Offset = a4;
        Overlapped.OffsetHigh = 0;

        if (requestType == NFL_REQUEST_TYPE_WRITE) {
            return (
                g_WriteFileEx(*a2, lpBuffer, nNumberOfBytes, &Overlapped, nfd_win32_IoCompletionRoutine) ==
                0);
        }

        BOOL result =
            g_ReadFileEx(*a2, lpBuffer, nNumberOfBytes, &Overlapped, nfd_win32_IoCompletionRoutine) == 0;

        return result;
    } else {
        return (BOOL) (CDECL_CALL(0x007A0030, a1, a2, requestType, a4, lpBuffer, nNumberOfBytes));
    }
}

int nflIoExecute(nflDriver *driver, nflCommand *ioCommand)
{
    TRACE("nflIoExecute");

    if constexpr (1)
    {
        //sp_log("handle = 0x%08X, lpBuffer = 0x%08X", *_a2->field_8, _a1->buffer->field_10);

        int v5, v7;

        unsigned numOfBytes;

        int v3 = ioCommand->field_18;

        if (v3 != 0)
        {
            if (v3 != 1)
            {
                return 8;
            }

            v5 = ioCommand->field_4;
            numOfBytes = ioCommand->field_14;
            v7 = 0;
        } else {
            uint32_t a4;
            v7 = nflBufferAlign(driver, ioCommand, ioCommand->field_0, &a4, &numOfBytes);
            if (v7 == -1) {
                return 3;
            }

            if (numOfBytes == 0) {
                driver->field_1C = 0;

                driver->field_24 = *ioCommand;

                driver->field_40 = NFD_IO_STATE_WORKDONE;
                return 0;
            }

            v5 = a4;
        }

        driver->field_44 = v7;
        driver->field_18 = v5;
        driver->field_20 = numOfBytes;

        char *lpBuffer = ioCommand->field_10;
        if (v7) {
            lpBuffer = driver->buffer->field_10;
        }

        BOOL result =
#if 0
        (driver->field_10->field_0)
#else
            nfd_win32_IoExecute
#endif
            (driver, ioCommand->fileHandle, ioCommand->field_18, v5, lpBuffer, numOfBytes);

        if (!result) {
            memcpy(&driver->field_24, ioCommand, sizeof(driver->field_24));
            driver->field_40 = NFD_IO_STATE_WORKING;
        }

        return result;
    } else {
        return CDECL_CALL(0x0079FDA0, driver, ioCommand);
    }
}

DWORD __stdcall sub_81DF20(LPVOID lpThreadParameter)
{
    if constexpr (1) {
        if ( byte_95518C )
        {
            int v1 {};
            int v2 {};
            DWORD LastError {};
            void (__stdcall *v7)(int, DWORD, DWORD) {};
            void (__stdcall *v8)(DWORD, DWORD, DWORD) {};
            int v3;
            BOOL v4;
            DWORD v5;

            int v9 {};
            BOOL v10;
            DWORD v11;
            DWORD v12 {};
            void (__stdcall *v13)(DWORD, DWORD, DWORD) {};
            uint32_t v14;
            void (__stdcall *v15)(DWORD, uint32_t, DWORD) {};
            DWORD *v16 {};

            DWORD NumberOfBytesRead {};

            while ( 1 )
            {
                WaitForSingleObject(dword_987500, 0xFFFFFFFF);
                if ( !dword_9874F8 )
                {
                    v16 = static_cast<DWORD *>(dword_9874FC);
LABEL_29:
                    dword_9874F8 = (int)v16;
                    if ( v16 )
                    {
                        auto v17 = v16[2];
                        if ( *(DWORD *)(v17 + 16) <= 1u ) {
                            SetFilePointer(*(HANDLE *)v17, *(DWORD *)(*(DWORD *)(v17 + 12) + 8), nullptr, 0);
                        }
                    }

                    goto LABEL_32;
                }

                v1 = *(DWORD *)(dword_9874F8 + 8);
                v2 = *(DWORD *)(v1 + 16);
                if ( v2 ) {
                    break;
                }

                v3 = *(DWORD *)(v1 + 24) - *(DWORD *)(v1 + 20);
                NumberOfBytesRead = 0;
                if ( dword_955190 < int(v3) ) {
                    v3 = dword_955190;
                }

                v4 = ReadFile(*(HANDLE *)v1, *(LPVOID *)(v1 + 8), v3, (LPDWORD)&NumberOfBytesRead, nullptr);
                v5 = NumberOfBytesRead + *(DWORD *)(v1 + 8);
                *(DWORD *)(v1 + 20) += NumberOfBytesRead;
                *(DWORD *)(v1 + 8) = v5;
                LastError = GetLastError();
                if ( v4 ) {
                    goto LABEL_22;
                }

                if ( LastError == ERROR_HANDLE_EOF )
                {
                    v7 = *(void (__stdcall **)(int, DWORD, DWORD))(v1 + 28);
                    *(DWORD *)(v1 + 16) = 4;
                    if ( !v7 ) {
                        goto LABEL_26;
                    }
LABEL_9:
                    v7(38, *(DWORD *)(v1 + 20), *(DWORD *)(v1 + 12));
                    **(DWORD **)(v1 + 12) = 0;
                    goto LABEL_32;
                }

                v8 = *(void (__stdcall **)(DWORD, DWORD, DWORD))(v1 + 28);
                *(DWORD *)(v1 + 16) = 3;
                if ( !v8 ) {
                    goto LABEL_26;
                }

                v8(LastError, *(DWORD *)(v1 + 20), *(DWORD *)(v1 + 12));
                **(DWORD **)(v1 + 12) = 0;
LABEL_32:
                ReleaseMutex(dword_987500);
                Sleep(dwMilliseconds);

                if ( !byte_95518C ) {
                    return 0;
                }
            }

            if ( v2 != 1 )
            {
                v16 = *(DWORD **)(dword_9874F8 + 12);
                goto LABEL_29;
            }

            v9 = *(DWORD *)(v1 + 24) - *(DWORD *)(v1 + 20);
            NumberOfBytesRead = 0;
            if ( dword_955190 < int(v9) ) {
                v9 = dword_955190;
            }

            v10 = WriteFile(*(HANDLE *)v1, *(LPCVOID *)(v1 + 8), v9, &NumberOfBytesRead, nullptr);
            v11 = NumberOfBytesRead + *(DWORD *)(v1 + 8);
            *(DWORD *)(v1 + 20) += NumberOfBytesRead;
            *(DWORD *)(v1 + 8) = v11;
            v12 = GetLastError();
            if ( v10 )
            {
LABEL_22:
                v14 = *(DWORD *)(v1 + 20);
                if ( v14 < *(DWORD *)(v1 + 24) && NumberOfBytesRead ) {
                    goto LABEL_32;
                }

                v15 = *(void (__stdcall **)(DWORD, unsigned int, DWORD))(v1 + 28);
                *(DWORD *)(v1 + 16) = 3;
                if ( v15 ) {
                    v15(0, v14, *(DWORD *)(v1 + 12));
                }

LABEL_26:
                **(DWORD **)(v1 + 12) = 0;
                goto LABEL_32;
            }

            if ( v12 != ERROR_HANDLE_EOF )
            {
                v13 = *(void (__stdcall **)(DWORD, DWORD, DWORD))(v1 + 28);
                *(DWORD *)(v1 + 16) = 3;
                if ( v13 ) {
                    v13(v12, *(DWORD *)(v1 + 20), *(DWORD *)(v1 + 12));
                }
LABEL_21:
                **(DWORD **)(v1 + 12) = 0;
                goto LABEL_32;
            }

            v7 = *(void (__stdcall **)(int, DWORD, DWORD))(v1 + 28);
            *(DWORD *)(v1 + 16) = 4;
            if ( !v7 ) {
                goto LABEL_21;
            }

            goto LABEL_9;
        }

        return 0;
    } else {
        DWORD (__stdcall *func)(LPVOID lpThreadParameter) = CAST(func, 0x0081DF20);
        return func(lpThreadParameter);
    }
}

void initializeFileHandlers(int *a1)
{
    if constexpr (1) {
        OSVERSIONINFOA VersionInformation {};
        memset(&VersionInformation, 0, sizeof(VersionInformation));
        VersionInformation.dwOSVersionInfoSize = 148;
        if ( GetVersionExA(&VersionInformation)
                && VersionInformation.dwPlatformId == 2
                && VersionInformation.dwMajorVersion >= 5 )
        {
            g_CreateFileA = CreateFileA;
            g_ReadFile = ReadFile;
            g_ReadFileEx = ReadFileEx;
            g_WriteFile = WriteFile;
            g_WriteFileEx = WriteFileEx;
            g_GetOverlappedResult = GetOverlappedResult;
            g_CancelIo = CancelIo;
            g_CloseHandle = CloseHandle;
        }
        else
        {
            g_CreateFileA = sub_81D790;
            g_ReadFile = sub_81D820;
            g_ReadFileEx = sub_81D950;
            g_WriteFile = sub_81DA70;
            g_WriteFileEx = sub_81DB90;
            g_GetOverlappedResult = sub_81DCF0;
            g_CancelIo = sub_81DCA0;
            g_CloseHandle = sub_81DE70;

            dword_987500 = CreateMutexA(nullptr, 0, nullptr);
            int ThreadId = 0;
            dword_9874F8 = (int)dword_9874FC;
            auto v1 = CreateThread(nullptr, 0, sub_81DF20, nullptr, 4u, (LPDWORD)&ThreadId);
            hThread = v1;

            if ( a1 )
            {
                SetThreadPriority(v1, *a1);
                dword_955190 = (unsigned __int64)*((float *)a1 + 1);
                dwMilliseconds = a1[2];
            }
            else
            {
                SetThreadPriority(v1, 0);
            }

            ResumeThread(hThread);
        }
    } else {
        void (*func)(int *) = CAST(func, 0x0081E130);
        func(a1);
    }
}

void releaseFileHandlers()
{
    if constexpr (1) {
    } else {
        CDECL_CALL(0x0081D700);
    }
}


void nfl_driver_patch() {
    //REDIRECT(0x0079FDDA, nfdBufferAlign);

    REDIRECT(0x0079EBEB, nflIoExecute);

    //set_vfunc(0x0094986C, sub_7A0030);
}
