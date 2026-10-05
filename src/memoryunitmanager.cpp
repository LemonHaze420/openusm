#include "memoryunitmanager.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"

#include <algorithm>
#include <direct.h>
#include <shlobj.h>
#include <cstdio>
#include <cstring>

namespace MemoryUnitManager {

VALIDATE_OFFSET(Container, field_200, 0x200);
VALIDATE_OFFSET(Container, field_220, 0x220);
VALIDATE_OFFSET(Container, field_240, 0x240);
VALIDATE_OFFSET(Container, field_248, 0x248);
VALIDATE_SIZE(Container, 0x288);

Var<Observer *> mObserver{0x00984818};

Var<eStatus> mLastError{0x009847CC};
namespace {
std::FILE *active_file = nullptr;

void close_active_file()
{
    if (active_file != nullptr) {
        std::fclose(active_file);
        active_file = nullptr;
    }
}

void finish_operation()
{
    const eOperation completed_operation = mCurrentOperation();
    close_active_file();
    mCurrentOperation() = OPERATION_NONE;
    mGameSave().Reset("");
    if (mObserver() != nullptr)
        mObserver()->Notify(completed_operation);
}
}  // namespace


bool get_path(const char *a1, const char *a2, char *out, unsigned int str_len)
{
    char documents[MAX_PATH]{};
    HMODULE shell = LoadLibraryA("shell32.dll");
    if (shell != nullptr) {
        using SHGetFolderPathA_t = HRESULT(WINAPI *)(HWND, int, HANDLE, DWORD, LPSTR);
        auto get_folder = bit_cast<SHGetFolderPathA_t>(GetProcAddress(shell, "SHGetFolderPathA"));
        if (get_folder != nullptr && SUCCEEDED(get_folder(nullptr, CSIDL_PERSONAL, nullptr, 0, documents))) {
            char path[MAX_PATH]{};
            std::snprintf(path, sizeof(path), "%s\\%s", documents, a1);
            if (std::strlen(path) >= str_len) {
                FreeLibrary(shell);
                return false;
            }
            std::strcpy(out, path);
        }
        FreeLibrary(shell);
    }

    if (documents[0] == '\0') {
        if (std::strlen(a2) >= str_len)
            return false;
        std::strcpy(out, a2);
    }
    return true;
}

void create_directory(const char *Source)
{
    char Dest[260]{};
    strncpy(Dest, Source, 260u);
    if (Dest[0]) {
        auto *v1 = Dest;
        do {
            if (*v1 == '\\') {
                *v1 = 0;
                _mkdir(Dest);
                *v1 = '\\';
            }
        } while (*++v1);
    }

    _mkdir(Dest);
}

void sub_7B11C0(char *a1)
{
    static char path[MAX_PATH]{};
    static bool initialized = false;
    if (!initialized) {
        get_path("Activision\\Ultimate Spider-Man\\", "Save\\", path, MAX_PATH);
        create_directory(path);
        initialized = true;
    }

    std::strcpy(a1, path);
}

Container::Container(const char *a2)
{
    std::memset(this, 0, sizeof(*this));
    Reset(a2);
}

void Container::Reset(const char *name)
{
    std::strcpy(field_248, name);
    field_240 = 0;
    field_244 = 0;
}

bool Container::AddFile(const char *name, char *buffer, unsigned int size)
{
    if (field_244 >= 8)
        return false;
    std::strcpy(field_0[field_244], name);
    field_200[field_244] = buffer;
    field_220[field_244] = size;
    ++field_244;
    return true;
}

void RegisterObserver(Observer *observer)
{
    mObserver() = observer;
}

void Initialize(uint32_t a1)
{
    if constexpr (STANDALONE_SYSTEM) {
        (void)a1;
        close_active_file();
        mObserver() = nullptr;
        mLastError() = STATUS_OK;
        mCurrentOperation() = OPERATION_NONE;
        new (&mGameSave()) Container{""};
    } else {
        CDECL_CALL(0x007B16D0, a1);
    }
}

bool Service()
{
    const eOperation operation = mCurrentOperation();
    if (operation == OPERATION_NONE)
        return false;
    if (active_file == nullptr)
        return StartOperation();

    auto &container = mGameSave();
    const unsigned int index = container.field_240;
    const size_t size = container.field_220[index];
    size_t transferred = 0;
    if (operation == OPERATION_LOAD) {
        transferred = std::fread(container.field_200[index], 1, size, active_file);
    } else if (operation == OPERATION_SAVE) {
        transferred = std::fwrite(container.field_200[index], 1, size, active_file);
    }
    close_active_file();

    if (transferred != size) {
        SetLastError(operation == OPERATION_SAVE ? STATUS_WRITE_ERROR : STATUS_ERROR);
        finish_operation();
        return false;
    }

    ++container.field_240;
    SetLastError(STATUS_OK);
    return StartOperation();
}

void SetLastError(eStatus a1)
{
    mLastError() = a1;
}

bool StartOperation()
{
    const eOperation operation = mCurrentOperation();
    if (operation == OPERATION_NONE)
        return false;
    if (active_file != nullptr)
        return true;

    auto &container = mGameSave();
    if (container.field_240 >= container.field_244) {
        finish_operation();
        return false;
    }

    char root[MAX_PATH]{};
    sub_7B11C0(root);
    char directory[MAX_PATH]{};
    std::snprintf(directory, sizeof(directory), "%s%s", root, container.field_248);
    if (operation == OPERATION_SAVE)
        create_directory(directory);

    char filename[MAX_PATH]{};
    std::snprintf(filename, sizeof(filename), "%s\\%s", directory, container.field_0[container.field_240]);
    active_file = std::fopen(filename, operation == OPERATION_LOAD ? "rb" : "wb");
    if (active_file == nullptr) {
        SetLastError(operation == OPERATION_LOAD ? STATUS_FILE_NOT_FOUND : STATUS_ERROR);
        finish_operation();
        return false;
    }
    return true;
}

eStatus GetLastError()
{
    return mLastError();
}

bool GetDiskInfo(DWORD *info)
{
    if (mCurrentOperation() != OPERATION_NONE) {
        SetLastError(STATUS_BUSY);
        return false;
    }

    ULARGE_INTEGER free_bytes{};
    ULARGE_INTEGER total_bytes{};
    ULARGE_INTEGER total_free{};
    if (!GetDiskFreeSpaceExA("C:\\", &free_bytes, &total_bytes, &total_free)) {
        SetLastError(STATUS_ERROR);
        return false;
    }

    info[0] = free_bytes.LowPart;
    info[1] = free_bytes.HighPart;
    info[2] = total_bytes.LowPart;
    info[3] = total_bytes.HighPart;
    info[4] = static_cast<DWORD>(std::min<unsigned long long>(free_bytes.QuadPart >> 10, 50001));
    info[5] = static_cast<DWORD>(std::min<unsigned long long>(total_bytes.QuadPart >> 10, 50001));
    info[6] = 1;
    SetLastError(STATUS_OK);
    return true;
}

int EnumerateSaveDirectories()
{
    char root[MAX_PATH]{};
    sub_7B11C0(root);
    char save_path[MAX_PATH]{};
    std::snprintf(save_path, sizeof(save_path), "%sSave", root);
    const DWORD attributes = GetFileAttributesA(save_path);
    if (attributes == INVALID_FILE_ATTRIBUTES || (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        return 0;

    int count = 0;
    for (int slot = 0; slot < 3; ++slot) {
        char filename[MAX_PATH]{};
        std::snprintf(filename, sizeof(filename), "%s\\Save%d", save_path, slot);
        FILE *file = std::fopen(filename, "ab+");
        if (file == nullptr)
            continue;
        std::fseek(file, 0, SEEK_END);
        if (std::ftell(file) == 0) {
            char zeros[0x4000]{};
            std::fwrite(zeros, 1, sizeof(zeros), file);
        }
        std::fclose(file);
        ++count;
    }
    return count;
}

eStatus LoadGame(const Container &container)
{
    if (mCurrentOperation() != OPERATION_NONE) {
        SetLastError(STATUS_BUSY);
        return GetLastError();
    }

    mGameSave() = container;
    mGameSave().field_240 = 0;
    mCurrentOperation() = OPERATION_LOAD;
    SetLastError(STATUS_OK);
    StartOperation();
    return GetLastError();
}

int LoadGameSync(const Container &container)
{
    LoadGame(container);
    while (mCurrentOperation() != OPERATION_NONE)
        Service();
    return static_cast<int>(GetLastError());
}

int SaveGame(const Container &container)
{
    if (mCurrentOperation() != OPERATION_NONE) {
        SetLastError(STATUS_BUSY);
        return static_cast<int>(GetLastError());
    }

    mGameSave() = container;
    mGameSave().field_240 = 0;
    mCurrentOperation() = OPERATION_SAVE;
    SetLastError(STATUS_OK);
    StartOperation();
    return static_cast<int>(GetLastError());
}

unsigned int GetGameSaveSize(unsigned int a1)
{
    return a1;
}

}  // namespace MemoryUnitManager
