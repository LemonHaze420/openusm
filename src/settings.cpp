#include "settings.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

#include "cstring"

VALIDATE_SIZE(Settings, 0x78);

Settings *& g_settings = var<Settings *>(0x00965ABC);

Settings::Settings(const char *Source, const char *a3) {
    strncpy(this->field_0, Source, 39u);
    strncpy(&this->field_0[40], a3, 79u);
}

int Settings::sub_81D010(const char *Source, int default_value) {
    if constexpr (1) {
        int result;

        DWORD Data;

        DWORD Type;

        DWORD cbData = 4;
        if (!this->sub_81CDC0(Source, &Type, (LPBYTE) &Data, &cbData) ||
            (result = Data, Type != REG_DWORD)) {
            result = default_value;
        }

        return result;

    } else {
        auto result = THISCALL(0x0081D010, this, Source, default_value);

        return result;
    }
}

bool Settings::sub_81CDC0(const char *Source,
                          OUT OPTIONAL LPDWORD lpType,
                          OUT OPTIONAL LPBYTE lpData,
                          IN OUT OPTIONAL LPDWORD lpcbData) {
    return (bool) THISCALL(0x0081CDC0, this, Source, lpType, lpData, lpcbData);
}

void Settings::sub_81CFA0(const char *a1, const char *a2, char *Type, DWORD cbData) {
    if constexpr (1) {
        char *v5 = Type;
        DWORD v6 = cbData;

        if (!this->sub_81CDC0(a1, (LPDWORD) &Type, (LPBYTE) Type, &cbData) && strlen(a2) < v6) {
            strcpy(v5, a2);
        }
    } else {
        THISCALL(0x0081CFA0, this, a1, a2, Type, cbData);
    }
}

bool Settings::sub_81CF80(const char *Source, BYTE Data)
{
    return this->sub_81CC80(Source, 4u, &Data, 4u);
}

bool Settings::sub_81CC80(const char *Source, DWORD dwType, BYTE *lpData, DWORD cbData)
{
    if constexpr (0)
    {}
    else
    {
        return THISCALL(0x0081CC80, this, Source, dwType, lpData, cbData);
    }
}

bool Settings::sub_81D050(const char *a2, bool a3)
{
    auto result = (bool) THISCALL(0x0081D050, this, a2, a3);

    return result;
}

void settings_patch() {
    {
        FUNC_ADDRESS(address, &Settings::sub_81D010);
        REDIRECT(0x005AC447, address);
    }

    {
        FUNC_ADDRESS(address, &Settings::sub_81D050);
        REDIRECT(0x005AC430, address);
    }
}
