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

int Settings::sub_81D010(const char *Source, int default_value)
{
    if constexpr (1)
    {
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
                          IN OUT OPTIONAL LPDWORD lpcbData)
{
    if constexpr (1)
    {
        char Dest[260] {};
        char v15[260] {};
        CHAR ValueName[260] {};

        char v13[4] {};

        constexpr int16_t word_883D78 {'\\'};

        auto result = this->sub_81CC00(Source, v15, 260, ValueName, 260);
        if ( result )
        {
            sprintf(Dest, "Software\\%s\\%s", this->field_0, &this->field_0[40]);
            if ( v15[0] )
            {
                char *v7 = &v13[3];

                while (*++v7) {
                    ;
                }

                *bit_cast<int16_t *>(v7) = word_883D78;
                auto v9 = strlen(v15) + 1;
                auto *v10 = &v13[3];

                while ( *++v10 ) {
                    ;
                }

                std::memcpy(v10, v15, v9);
            }

            HKEY phkResult {};
            bool v12 = false;
            if ( RegOpenKeyA(HKEY_CURRENT_USER, Dest, &phkResult) == ERROR_SUCCESS ) {
                if ( RegQueryValueExA(phkResult, ValueName, nullptr, lpType, lpData, lpcbData) == ERROR_SUCCESS ) {
                    v12 = true;
                }

                RegCloseKey(phkResult);
            }

            if ( !v12 )
            {
                if ( RegOpenKeyA(HKEY_LOCAL_MACHINE, Dest, &phkResult) == ERROR_SUCCESS )
                {
                    if ( RegQueryValueExA(phkResult, ValueName, nullptr, lpType, lpData, lpcbData) == ERROR_SUCCESS ) {
                        v12 = true;
                    }

                    RegCloseKey(phkResult);
                }
            }

            return v12;
        }
        return result;
    } else {
        bool (__fastcall *func)(void *, void *edx,
                          const char *,
                          OUT OPTIONAL LPDWORD,
                          OUT OPTIONAL LPBYTE,
                          IN OUT OPTIONAL LPDWORD) = CAST(func, 0x0081CDC0);
        return func(this, nullptr, Source, lpType, lpData, lpcbData);
    }
}

bool Settings::sub_81CC00(const char *Source, char *Dest, int a3, char *a4, int a5)
{
    int v6 = -1;
    if ( Source == nullptr ) {
        return false;
    }

    auto v7 = *Source;
    int i;
    for ( i = 0; v7; ++i )
    {
        if ( v7 == '\\' ) {
            v6 = i;
        }

        v7 = Source[i + 1];
    }

    auto v9 = i - v6 - 1;
    if ( !v9 || a5 <= v9 ) {
        return false;
    }

    Dest[0] = 0;
    if ( v6 != -1 && a3 > v6 )
    {
        strncpy(Dest, Source, v6);
        Dest[v6] = 0;
    }

    auto *v10 = bit_cast<char *>(&Source[v6 + 1]);
    int v11 = a4 - v10;
    char v12;

    do
    {
        v12 = v10[0];
        v10[v11] = v10[0];
        ++v10;
    }
    while ( v12 );

    return true;
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
