#pragma once

#include "variable.h"

#include <windows.h>

struct Settings {
    char field_0[120];

    //0x0081CBD0
    Settings(const char *Source, const char *a3);

    int sub_81D010(const char *Source, int default_value);

    void sub_81CFA0(const char *a1, const char *a2, char *Type, DWORD cbData);

    bool sub_81CDC0(const char *Source, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);

    bool sub_81CF80(const char *Source, BYTE Data);

    bool sub_81CC80(const char *Source, DWORD dwType, BYTE *lpData, DWORD cbData);

    bool sub_81D050(const char *a2, bool a3);

    static inline float &GameSoundVolume = var<float>(0x00965958);

    static inline float &MusicVolume = var<float>(0x00965AC0);

    static inline int &SoundMode = var<int>(0x00961924);

    static inline bool &InvertCameraV = var<bool>(0x00961914);

    static inline bool &InvertCameraH = var<bool>(0x009659A0);

    static inline bool &MouseLook = var<bool>(0x00965BF4);
};

extern Settings *& g_settings;

extern void settings_patch();
