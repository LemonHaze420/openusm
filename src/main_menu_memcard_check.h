#pragma once

#include "femenu.h"
#include "mAvlTree.h"
#include "mstring.h"
#include "memoryunitmanager.h"


struct FrontEndMenuSystem;
struct FEMultiLineText;
struct FEText;
struct PanelAnimFile;
struct PanelQuad;
struct string_hash_entry;

struct main_menu_memcard_check : FEMenu {
    enum dialog_state : int {
        DIALOG_NONE = 0,
        DIALOG_NO_SAVE = 1,
        DIALOG_HAS_SAVE = 2,
        DIALOG_CHECKING = 5,
        DIALOG_LOAD_CORRUPT = 14,
        DIALOG_OPERATION_FAILED = 15,
        DIALOG_INSUFFICIENT_SPACE = 16,
    };

    MemoryUnitManager::InsertRemoveObserver field_2C;
    PanelQuad *field_30[27];
    PanelQuad *field_9C[4];
    PanelQuad *field_AC[3];
    PanelAnimFile *field_B8;
    PanelAnimFile *field_BC;
    PanelAnimFile *field_C0;
    PanelAnimFile *field_C4;
    PanelAnimFile *field_C8;
    PanelAnimFile *field_CC;
    PanelAnimFile *field_D0;
    FEText *field_D4[3];
    FEText *field_E0[3];
    FEMultiLineText *field_EC;
    FEMultiLineText *field_F0;
    FEMultiLineText *field_F4;
    bool field_F8;
    char field_F9[3];
    float field_FC;
    int field_100;
    int field_104;
    dialog_state field_108;
    mAvlTree<string_hash_entry> field_10C;
    int field_11C;
    bool field_120;
    char field_121[7];
    bool field_128;
    bool field_129;
    char field_12A[2];
    mString field_12C[6];
    mString field_18C;
    FrontEndMenuSystem *field_19C;

    main_menu_memcard_check(FEMenuSystem *a2, int a4, int a5);

    void _Init();

    void LoadMemoryCard();

    void SetDialogMessage();
    void SetUpDialogBox(dialog_state state);

    void OperationFailed(MemoryUnitManager::eOperation operation, MemoryUnitManager::eStatus status);


    void UpdateText();

    void Draw();

    void Update(Float delta_time);

    void OnActivate();

    void OnCross(int controller);

    void OnSuccessfulLoad();
};
