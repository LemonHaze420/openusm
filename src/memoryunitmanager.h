#pragma once

#include "variable.h"

#include <windef.h>

namespace MemoryUnitManager {

enum eOperation {
    OPERATION_NONE = 0,
    OPERATION_LOAD = 1,
    OPERATION_SAVE = 2,
    OPERATION_DELETE = 3,
    OPERATION_FORMAT = 4,
};

enum eStatus {
    STATUS_BUSY = -17,
    STATUS_FILE_NOT_FOUND = -13,
    STATUS_LOAD_CORRUPT = -12,
    STATUS_WRITE_ERROR = -3,
    STATUS_ERROR = -1,
    STATUS_OK = 0,
};

struct Container {
    char field_0[8][64];
    char *field_200[8];
    unsigned int field_220[8];
    unsigned int field_240;
    unsigned int field_244;
    char field_248[64];

    Container() : Container("") {}

    //0x007B1160
    Container(const char *a2);

    void Reset(const char *name);

    bool AddFile(const char *name, char *buffer, unsigned int size);

    char *GetGameName()
    {
        return this->field_248;
    }
};

struct Observer;

struct ObserverVTable {
    using callback_fn = void(__fastcall *)(Observer *, void *, eOperation);

    callback_fn Callback;
};

struct Observer {
    ObserverVTable *m_vtbl;

    void Notify(eOperation operation)
    {
        m_vtbl->Callback(this, nullptr, operation);
    }
};

struct InsertRemoveObserver;

struct InsertRemoveObserverVTable {
    using callback_fn = void(__stdcall *)(InsertRemoveObserver *, int);

    callback_fn Callback;
};

struct InsertRemoveObserver {
    InsertRemoveObserverVTable *m_vtbl;
};

extern void RegisterObserver(Observer *a1);

extern void Initialize(uint32_t a1);

extern bool Service();

extern void SetLastError(eStatus a1);

extern eStatus GetLastError();

extern void sub_7B11C0(char *path);

extern bool GetDiskInfo(DWORD *info);

extern int EnumerateSaveDirectories();

extern bool StartOperation();

extern eStatus LoadGame(const Container &a1);

extern int LoadGameSync(const Container &a1);

//0x007B1720
extern unsigned int GetGameSaveSize(unsigned int a1);

extern Var<Observer *> mObserver;

extern Var<eStatus> mLastError;

//0x007B1FF0
extern int SaveGame(const Container &a1);

inline Var<eOperation> mCurrentOperation{0x0098481C};

inline Var<Container> mGameSave{0x00984828};

}  // namespace MemoryUnitManager
