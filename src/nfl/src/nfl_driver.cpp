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

#else

static nflDriver *& dword_984550 = []() -> auto & {
    static nflDriver * g_dword_984550 {};
    return g_dword_984550;
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
#ifndef TEST_CASE

    Var<decltype(&CancelIo)> p_CancelIo{0x0098750C};
#else

    static decltype(&CancelIo) g_p_CancelIo{&CancelIo};
    Var p_CancelIo{&g_p_CancelIo};

#endif

    return p_CancelIo()(*a1) == 0;
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

        auto & g_WriteFileEx = var<BOOL(__stdcall *)(HANDLE, LPCVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE)>(0x009551A8);
        auto & g_ReadFileEx = var<BOOL(__stdcall *)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE)>(0x009551A0);

#else

        _OVERLAPPED & Overlapped = []() -> auto & {
            static _OVERLAPPED g_Overlapped {};
            return g_Overlapped;
        }();

        auto & g_WriteFileEx = []() -> auto & {
            static BOOL (__stdcall *g_WriteFileEx1)(HANDLE, LPCVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
            return g_WriteFileEx1;
        }();

        auto & g_ReadFileEx = []() -> auto & {
            static BOOL (__stdcall *g_ReadFileEx1)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
            return g_ReadFileEx1;
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

void nfl_driver_patch() {
    //REDIRECT(0x0079FDDA, nfdBufferAlign);

    REDIRECT(0x0079EBEB, nflIoExecute);

    //set_vfunc(0x0094986C, sub_7A0030);
}
