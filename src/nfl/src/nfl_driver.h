#pragma once

#include <cstdint>
#include <windows.h>

#include "win32/nfl_win32_driver.h"

struct nflDriver;
struct nflCommand;

//0x0079FC50
extern int nflBufferAlign(
    nflDriver *driver, nflCommand *ioCommand, uint32_t a3, uint32_t *a4, uint32_t *a5);

//0x0079FDA0
extern int nflIoExecute(nflDriver *driver, nflCommand *ioCommand);

extern BOOL nfd_win32_IoExecute(
    nflDriver *a1, HANDLE *a2, nflRequestType, uint32_t a4, LPVOID lpBuffer, uint32_t nNumberOfBytes);

extern BOOL sub_7A00A0(HANDLE *a1);

extern int sub_7A00C0();

//0x0081E130
extern void initializeFileHandlers(int *);

//0x0081D700
extern void releaseFileHandlers();

extern void nfl_driver_patch();
